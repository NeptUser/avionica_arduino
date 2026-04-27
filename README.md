# Aviônica Arduino — Relatório Técnico

**Disciplina:** Sistemas Embarcados  
**Atividade:** Trabalho Complementar — Dinâmica 2 (Projeto Embarcado Livre)

---

## 1. Descrição do Problema

O sistema implementa a aviônica embarcada de um foguete experimental, responsável por:
- Aquisição contínua de dados inerciais e barométricos durante o voo
- Armazenamento local em memória flash não-volátil
- Transmissão de telemetria em tempo real para estação de solo via rádio LoRa
- Detecção automática das fases do voo através de máquina de estados

O sistema opera sem sistema operacional (bare-metal), com controle de temporização via registradores do ATmega328P.

---

## 2. Lista de Componentes

| Componente | Especificação |
|---|---|
| Microcontrolador | Arduino Nano (ATmega328P, 16MHz, 2KB RAM, 32KB Flash) |
| IMU | MPU-6050 — acelerômetro ±8g + giroscópio ±500°/s, I²C 0x68 |
| Barômetro | BMP280 — pressão 300-1100hPa, altitude relativa, I²C 0x76 |
| Rádio | LoRa E32-915MHz — UART 9600bps, alcance ~3km |
| Memória | W25Q128 — Flash SPI 16MB, 100.000 ciclos de escrita |

---

## 3. Montagem

Diagrama de conexões:

```
Arduino Nano
├── I²C  (A4=SDA, A5=SCL) ──── MPU-6050 (0x68)
│                          └── BMP280   (0x76)
├── SPI  (D11=MOSI, D12=MISO, D13=SCK)
│   └── D10 (CS) ───────────── W25Q128
├── SoftwareSerial
│   ├── D3 (TX) ────────────── LoRa E32 RX
│   └── D4 (RX) ────────────── LoRa E32 TX
├── D5 ─────────────────────── LoRa E32 AUX
├── D6 ─────────────────────── LoRa E32 M0
├── D7 ─────────────────────── LoRa E32 M1
└── D2 (INT0) ───────────────── MPU-6050 INT
```

---

## 4. Esquemático

> Diagrama elétrico a ser inserido via Tinkercad.

---

## 5. Código

O projeto é organizado nos seguintes arquivos:

| Arquivo | Responsabilidade |
|---|---|
| `globals.hpp` | Pinout, thresholds, structs e enum de estados |
| `sensors.hpp/.cpp` | Leitura do MPU-6050 e BMP280 via I²C |
| `storage.hpp/.cpp` | Gravação e dump na flash W25Q128 via SPI |
| `telemetry.hpp/.cpp` | Transmissão de pacotes via LoRa E32 |
| `statemachine.hpp/.cpp` | Máquina de estados do voo |
| `avionica_arduino.ino` | Loop principal, ISRs e configuração de registradores |

### Bibliotecas utilizadas
- `Adafruit MPU6050`
- `Adafruit BMP280`
- `Adafruit Unified Sensor`
- `SPIMemory`
- `SoftwareSerial` (built-in)

### Estruturas de dados centrais

```cpp
// Registro completo gravado na flash
struct DadosVoo {
    float acelX, acelY, acelZ;   // m/s²
    float giroX, giroY, giroZ;   // °/s
    float pressao;                // hPa
    float temperatura;            // °C
    float altitude;               // metros
    unsigned long timestamp;      // millis()
    EstadoVoo estado;             // fase do voo
};

// Pacote compacto transmitido pelo LoRa
struct PacoteTelemetria {
    float acelX, acelY, acelZ;
    float altitude;
    float pressao;
    unsigned long timestamp;
    EstadoVoo estado;
};
```

### Capacidade de armazenamento

```
sizeof(DadosVoo) ≈ 45 bytes
Capacidade W25Q128 = 16.777.216 bytes
Registros máximos  = ~372.827 registros
Frequência de gravação = 25Hz
Tempo máximo de voo  = ~14.913s ≈ 4,1 horas
```

### Fases do voo (máquina de estados)

```
ARMADO ──(acel > 3g)──► ALTA_ENERGIA
  ──(acel < 1.2g)──► BAIXA_ENERGIA
    ──(alt < max - 5m)──► QUEDA
      ──(alt < 5m)──► ATERRISSADO
```

---

## 6. Descrição dos Registradores

### 6.1 Sensores — Interrupção Externa INT0 (MPU-6050)

O MPU-6050 sinaliza disponibilidade de nova amostra através do pino `INT` (D2). Em vez de verificar continuamente (*polling*), configurou-se a interrupção externa `INT0` do ATmega328P para reagir automaticamente à borda de subida desse sinal.

| Registrador | Configuração | Função |
|---|---|---|
| `EICRA` | `ISC01=1, ISC00=1` | Disparo na borda de subida (LOW→HIGH) |
| `EIMSK` | `INT0=1` | Habilita a interrupção no pino D2 |

```cpp
EICRA |= (1 << ISC01) | (1 << ISC00); // borda de subida
EIMSK |= (1 << INT0);                  // habilita INT0
```

**Motivação:** a ISR apenas seta uma flag (`leituraPendente = true`), custando ciclos mínimos. O loop principal consome a flag e executa a leitura, garantindo que nenhuma amostra seja perdida sem bloquear o processador.

### 6.2 Armazenamento — SPI via Biblioteca

O módulo W25Q128 utiliza o periférico SPI nativo do ATmega328P (pinos fixos D11, D12, D13), gerenciado pela biblioteca `SPIMemory`. O controle direto via registradores (`SPCR`, `SPSR`) é feito internamente pela biblioteca, que configura o SPI em modo mestre, polaridade 0, fase 0 e clock de 8MHz.

O pino `CS` (D10) é controlado pela biblioteca, que usa `digitalWrite` internamente. A decisão de não reimplementar o driver SPI via registradores é justificada pela complexidade do protocolo de comandos do W25Q128 (erase, page program, status polling), que está encapsulada e testada na biblioteca.

**Custo de gravação:** 45 bytes × 8 bits / 8MHz = **720 clocks por registro**.

### 6.3 Transmissão — Controle do LoRa E32 via Registradores

Os pinos de controle do módulo LoRa E32 (M0, M1, AUX) são configurados e operados diretamente via registradores do porto D, sem uso de `digitalWrite` ou `pinMode`.

| Registrador | Configuração | Função |
|---|---|---|
| `DDRD` | `bit 6 = 1` | M0 (D6) como saída |
| `DDRD` | `bit 7 = 1` | M1 (D7) como saída |
| `DDRD` | `bit 5 = 0` | AUX (D5) como entrada |
| `PORTD` | `bit 5 = 1` | Pull-up interno no AUX |
| `PORTD` | `bits 6,7 = 0` | M0=0, M1=0 → modo normal |
| `PIND` | `bit 5` | Polling do AUX antes de transmitir |

```cpp
DDRD |=  (1 << 6) | (1 << 7); // M0, M1 → saída
DDRD &= ~(1 << 5);             // AUX → entrada
PORTD |=  (1 << 5);            // pull-up no AUX
PORTD &= ~(1 << 6) | (1 << 7);// M0=0, M1=0 (modo normal)

// Polling antes de transmitir (3 clocks por iteração)
while (!(PIND & (1 << 5)));
```

**Motivação:** o pino AUX indica quando o módulo está ocupado. Monitorá-lo por polling via `PIND` custa apenas **3 clocks por iteração** (instruções `IN` + `ANDI` + `BREQ`), desprezível quando o módulo já está livre.

**Ausência de ISR no AUX:** o sistema opera exclusivamente como transmissor. Alocar `INT0` ao AUX comprometeria a aquisição determinística do MPU-6050, que tem prioridade máxima. A recepção de comandos pode ser implementada em versão futura com hardware de maior capacidade.

**Custo de transmissão:** 25 bytes × 10 bits / 9600bps = **26ms = ~416.666 clocks**.

### 6.4 Temporização — Timer1 em Modo CTC

O loop principal é cadenciado pelo Timer1 do ATmega328P em modo CTC (*Clear Timer on Compare Match*), gerando uma base de tempo de 100Hz sem uso de `delay()` ou `millis()`.

| Registrador | Configuração | Função |
|---|---|---|
| `TCCR1A` | `0x00` | Desativa PWM, modo CTC ativo via TCCR1B |
| `TCCR1B` | `WGM12=1, CS12=1` | Modo CTC + prescaler 256 |
| `OCR1A` | `624` | Valor de comparação → 100Hz |
| `TIMSK1` | `OCIE1A=1` | Habilita interrupção por comparação |

```cpp
// f = 16.000.000 / (256 × (624+1)) = 100Hz
TCCR1A = 0x00;
TCCR1B = (1 << WGM12) | (1 << CS12);
OCR1A  = 624;
TIMSK1 = (1 << OCIE1A);
```

**Motivação:** o modo CTC garante periodicidade exata independente do tempo gasto no loop. A ISR incrementa um contador de ticks, e o loop usa divisores para derivar as frequências de operação:

```
100Hz (todo tick)  → avalia máquina de estados
 25Hz (tick % 4)   → readData() + saveData()  (~5.200 clocks)
  5Hz (tick % 20)  → sendPacket()             (~416.666 clocks)
```

As frequências foram escolhidas para que nenhum ciclo exceda seu período disponível, eliminando risco de sobreposição sem necessidade de RTOS.

---

## 7. Resultados e Discussão

### Validação do funcionamento

O sistema foi projetado e validado conceitualmente com base nas especificações dos componentes. A arquitetura de software foi estruturada para garantir:

- **Determinismo temporal:** Timer1 em CTC garante cadência exata de 100Hz
- **Sem perda de amostras:** ISR do MPU-6050 sinaliza disponibilidade de dado antes da leitura
- **Sem sobreposição de operações:** divisores de frequência garantem que gravação (5.200 clocks) e transmissão (416.666 clocks) nunca ocorram no mesmo ciclo
- **Capacidade de armazenamento:** ~372.827 registros — suficiente para mais de 4 horas de voo a 25Hz

### Limitações encontradas

- **SoftwareSerial e interrupções:** a biblioteca `SoftwareSerial` desabilita interrupções durante transmissão, o que pode causar perda de ticks do Timer1 nos ~26ms de cada envio LoRa. Solução futura: usar hardware UART dedicado
- **Contador de registros volátil:** o contador de gravações da flash é mantido em RAM e zerado ao resetar o Arduino. Uma queda de energia durante o voo perderia a referência de posição
- **Thresholds fixos:** os valores de detecção de lançamento e apogeu são definidos em tempo de compilação. Idealmente seriam configuráveis via telemetria antes do voo

### Melhorias possíveis

- Implementar structs separadas para MPU e BMP com timestamps independentes, permitindo leituras assíncronas e aproveitamento da taxa de 100Hz do MPU
- Gravar o contador de registros na própria flash para sobreviver a resets
- Adicionar receptor de comandos via LoRa para configuração remota dos thresholds
- Migrar para Arduino Mega ou STM32 para dispor de múltiplos pinos de interrupção e UARTs dedicadas