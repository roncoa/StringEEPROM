# Libreria StringEEPROM

Una libreria per la gestione efficiente di stringhe multiple nella memoria EEPROM per piattaforme Arduino ed ESP32.

## Caratteristiche

- Memorizza multiple stringhe in EEPROM con allocazione dinamica
- Interfaccia di comando tramite Seriale per uso interattivo
- Output di debug configurabile
- Numero massimo di stringhe configurabile
- Implementazione efficiente della memoria usando la memoria flash per le stringhe
- Compatibile sia con Arduino che con ESP32

## Installazione

1. Scarica l'archivio della libreria
2. Estrailo nella cartella libraries di Arduino
3. Riavvia l'IDE di Arduino

## Formato di Memorizzazione

La libreria utilizza il seguente formato per memorizzare le stringhe in EEPROM:
```
[len1][data1][len2][data2]...[255]
```
dove:
- `len` = byte lunghezza (0-254)
- `data` = contenuto stringa
- `255` = terminatore che marca la fine di tutti i dati

## Riferimento della Classe

### Costruttore

```cpp
StringEEPROM()
```
Crea una nuova istanza di StringEEPROM con debug disabilitato e nessun limite al numero di stringhe.

### Metodi di Configurazione

```cpp
void setDebug(bool enable)
```
Abilita o disabilita i messaggi di debug sulla Seriale.

```cpp
bool isDebugEnabled() const
```
Restituisce lo stato attuale del debug.

```cpp
void setMaxStrings(int max)
```
Imposta il numero massimo di stringhe che possono essere memorizzate. Usa -1 per illimitato (default).

```cpp
int getMaxStrings() const
```
Ottiene il limite attuale di stringhe massime.

### Metodi Principali

```cpp
void begin(uint32_t baudRate = 115200)
```
Inizializza la libreria e la comunicazione Seriale.
- `baudRate`: Baud rate opzionale per la Seriale (default 115200)

```cpp
bool writeString(uint8_t n, const char* data)
```
Scrive una stringa nella posizione specificata.
- `n`: Posizione (indice base 1)
- `data`: Stringa da scrivere
- Restituisce: true se successo, false altrimenti

```cpp
int readString(uint8_t n, char* buffer, int maxLength)
```
Legge una stringa dalla posizione specificata.
- `n`: Posizione da leggere (indice base 1)
- `buffer`: Buffer dove memorizzare la stringa
- `maxLength`: Lunghezza massima del buffer
- Restituisce: Lunghezza della stringa o -1 se non trovata

```cpp
int check()
```
Controlla la validità del contenuto EEPROM.
- Restituisce: Numero di stringhe memorizzate, o -1 se il contenuto EEPROM non è valido

```cpp
void init()
```
Inizializza EEPROM scrivendo il terminatore all'inizio.

### Metodi di Debug

```cpp
void debugPrint(const __FlashStringHelper* message)
void debugPrintln(const __FlashStringHelper* message)
```
Stampa messaggio di debug dalla memoria flash, con o senza newline.

```cpp
void debugPrintValue(const __FlashStringHelper* message, int value)
void debugPrintlnValue(const __FlashStringHelper* message, int value)
```
Stampa messaggio di debug con valore intero, con o senza newline.

```cpp
void debugPrintChar(const char* message)
void debugPrintlnChar(const char* message)
```
Stampa messaggio di debug dalla RAM, con o senza newline.

### Metodi Interfaccia Seriale

```cpp
void handleSerial()
```
Processa i comandi Seriali. Deve essere chiamato nel loop().

```cpp
void showAllStrings()
```
Visualizza tutte le stringhe memorizzate con le loro posizioni.

```cpp
void printHelp()
```
Visualizza i comandi Seriali disponibili.

## Comandi Seriali

La libreria fornisce un'interfaccia interattiva tramite Seriale con i seguenti comandi:

- `N=stringa` - Scrive "stringa" nella posizione N (es. "1=Ciao")
- `?` - Mostra tutte le stringhe
- `#` - Mostra il numero di stringhe
- `!` - Inizializza EEPROM (richiede conferma)
- `h` - Mostra messaggio di aiuto

## Esempio di Utilizzo

Uso base:
```cpp
#include <StringEEPROM.h>

StringEEPROM eeprom;

void setup() {
  // Opzionale: imposta numero massimo di stringhe
  eeprom.setMaxStrings(10);
  
  // Opzionale: abilita messaggi di debug
  eeprom.setDebug(true);
  
  // Inizializza con baud rate di default (115200)
  eeprom.begin();
}

void loop() {
  // Gestisce i comandi Seriali
  eeprom.handleSerial();
}
```

## Considerazioni sulla Memoria

- Ogni stringa richiede lunghezza + 1 byte di spazio EEPROM (byte lunghezza + contenuto stringa)
- La libreria usa la memoria flash per le stringhe costanti per risparmiare RAM
- La lunghezza massima della stringa è 254 byte a causa delle limitazioni del byte di lunghezza
- Lo spazio EEPROM è utilizzato efficientemente con allocazione dinamica

## Compatibilità

- Testato su piattaforme Arduino (AVR)
- Testato su piattaforme ESP32

## Limitazioni

- Lunghezza massima stringa 254 byte
- Posizioni stringhe basate su indice 1
- Il contenuto della stringa non può contenere il valore byte 255 (usato come terminatore)
- L'EEPROM virtuale ESP32 richiede commit() dopo le modifiche

## Licenza

Questa libreria è rilasciata sotto licenza MIT.
