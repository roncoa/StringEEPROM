import serial
import serial.tools.list_ports
import time
import threading
import tkinter as tk
from tkinter import ttk, scrolledtext
import os
import sys

class SerialApp:
   def __init__(self, root):
       self.root = root
       self.root.title("StringEEPROM Tool v1.0.0")
       
       # Imposta le dimensioni iniziali e minime della finestra principale
       self.root.geometry("500x400")  # Dimensioni iniziali
       self.root.minsize(500, 400)    # Dimensioni minime
       
       self.serial_port = None
       self.reading = False
       self.entries = []
       
       root.columnconfigure(0, weight=1)
       root.rowconfigure(0, weight=1)
       
       self.main_frame = ttk.Frame(root, padding="10")
       self.main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
       self.main_frame.columnconfigure(1, weight=1)
       self.main_frame.rowconfigure(3, weight=1)
       
       serial_frame = ttk.Frame(self.main_frame)
       serial_frame.grid(row=0, column=0, columnspan=2, sticky=tk.W)
       
       ttk.Label(serial_frame, text="Porta:").pack(side=tk.LEFT, padx=5)
       self.port_var = tk.StringVar()
       self.port_combo = ttk.Combobox(serial_frame, textvariable=self.port_var, width=15)
       self.port_combo.pack(side=tk.LEFT, padx=5)
       
       ttk.Label(serial_frame, text="Baud Rate:").pack(side=tk.LEFT, padx=5)
       self.baud_var = tk.StringVar(value="115200")
       self.baud_combo = ttk.Combobox(serial_frame, textvariable=self.baud_var, 
                                     values=["9600", "19200", "38400", "57600", "115200"],
                                     width=10)
       self.baud_combo.pack(side=tk.LEFT, padx=5)

       # Aggiungi il pulsante info
       info_img = "⍰"
       self.info_btn = ttk.Button(serial_frame, text=info_img, width=3, 
                               command=self.show_info)
       self.info_btn.pack(side=tk.LEFT, padx=5)
       
       btn_frame = ttk.Frame(self.main_frame)
       btn_frame.grid(row=1, column=0, columnspan=2, sticky=tk.W, pady=5)
       
       # Aggiungi l'indicatore di stato
       self.status_indicator = tk.Canvas(btn_frame, width=16, height=16, bg=self.root['bg'], highlightthickness=0)
       self.status_indicator.pack(side=tk.LEFT, padx=5)
       self.status_circle = self.status_indicator.create_oval(2, 2, 14, 14, fill='red')
       
       self.connect_btn = ttk.Button(btn_frame, text="Connetti", command=self.toggle_connection)
       self.connect_btn.pack(side=tk.LEFT, padx=5)
       
       self.refresh_btn = ttk.Button(btn_frame, text="Refresh", command=self.send_question)
       self.refresh_btn.pack(side=tk.LEFT, padx=5)
       
       self.entries_frame = ttk.Frame(self.main_frame)
       self.entries_frame.grid(row=2, column=0, columnspan=2, sticky=(tk.W, tk.E))
       
       self.log_text = scrolledtext.ScrolledText(self.main_frame, width=40, height=10, state='disabled')
       self.log_text.grid(row=3, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), pady=5)
       
       send_frame = ttk.Frame(self.main_frame)
       send_frame.grid(row=4, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=5)
       send_frame.columnconfigure(1, weight=1)
       
       ttk.Label(send_frame, text="Custom:").pack(side=tk.LEFT, padx=5)
       self.custom_entry = ttk.Entry(send_frame)
       self.custom_entry.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=5)
       self.custom_entry.bind('<Return>', lambda e: self.send_custom())
       ttk.Button(send_frame, text="Invia", command=self.send_custom).pack(side=tk.LEFT, padx=5)
       
       self.update_ports()

   def show_info(self):
       info_window = tk.Toplevel(self.root)
       info_window.title("Informazioni")
       info_window.geometry("400x250")
       
       # Impedisce il ridimensionamento della finestra
       info_window.resizable(False, False)
       
       # Rendi la finestra modale (blocca l'interazione con la finestra principale)
       info_window.transient(self.root)
       info_window.grab_set()
       
       # Contenuto
       info_text = """StringEEPROM Tool v1.0.0
       
Questo tool permette di leggere e scrivere stringhe su EEPROM.

Utilizzo:
- Seleziona la porta COM appropriata
- Imposta il baud rate (default 115200)
- Usa il file 'StringEEPROM_tool.cfg' per personalizzare le etichette
- Premi 'Refresh' per aggiornare i valori

by roncoa@gmail.com"""
       
       label = ttk.Label(info_window, text=info_text, justify=tk.LEFT, padding=10)
       label.pack(expand=True, fill='both')
       
       # Pulsante chiudi
       ttk.Button(info_window, text="Chiudi", 
                  command=info_window.destroy).pack(pady=10)

   def log_message(self, message):
       self.log_text.config(state='normal')
       self.log_text.insert(tk.END, message + "\n")
       self.log_text.see(tk.END)
       self.log_text.config(state='disabled')

   def create_entries(self, num):
       try:
           # Se è un exe compilato, usa sys._MEIPASS
           if getattr(sys, 'frozen', False):
               application_path = os.path.dirname(sys.executable)
           else:
               application_path = os.path.dirname(os.path.abspath(__file__))
           config_path = os.path.join(application_path, 'StringEEPROM_tool.cfg')
     #      self.log_message(f"Cerco il file in: {config_path}")
           with open(config_path, 'r') as f:
               labels = f.read().splitlines()
               # Sostituisci righe vuote con numeri
               labels = [str(i+1) if not label.strip() else label for i, label in enumerate(labels)]
               self.log_message("File StringEEPROM_tool.cfg trovato e caricato")
               while len(labels) < num:
                   labels.append(str(len(labels) + 1))
       except FileNotFoundError:
           self.log_message("File StringEEPROM_tool.cfg non trovato, uso numeri progressivi")
           labels = [str(i+1) for i in range(num)]
       for widget in self.entries_frame.winfo_children():
           widget.destroy()
       self.entries.clear()
       
       # Configura la colonna 1 per espandersi
       self.entries_frame.columnconfigure(1, weight=1)
       
       ttk.Frame(self.entries_frame, height=10).grid(row=0, column=0, columnspan=3, sticky=(tk.W, tk.E))
       for i in range(num):
           ttk.Label(self.entries_frame, text=f"{labels[i]}:").grid(row=i+1, column=0, padx=5, pady=2)
           entry = ttk.Entry(self.entries_frame)
           entry.grid(row=i+1, column=1, sticky=(tk.W, tk.E), padx=5, pady=2)
           send_btn = ttk.Button(self.entries_frame, text="Invia", command=lambda x=i: self.send_entry(x))
           send_btn.grid(row=i+1, column=2, padx=5, pady=2)
           self.entries.append(entry)

   def process_response(self, data):
       if data.startswith("Number of strings:"):
           try:
               num = int(data.split(":")[1].strip())
               self.create_entries(num)
               self.log_message(f"Create {num} entries")
           except ValueError:
               self.log_message("Errore nel parsing del numero")
       elif data.startswith("String read from position"):
           try:
               position = int(data.split("position")[1].split("=")[0].strip())
               value = data.split("=")[1].strip()
               self.entries[position-1].delete(0, tk.END)
               self.entries[position-1].insert(0, value)
           except (ValueError, IndexError) as e:
               self.log_message(f"Errore nel parsing: {str(e)}")
               
   def update_ports(self):
       ports = [port.device for port in serial.tools.list_ports.comports()]
       self.port_combo['values'] = ports
       if ports:
           self.port_combo.set(ports[0])
       else:
           self.port_combo.set('')  # Pulisce il valore visualizzato
           self.log_message("Nessuna porta seriale disponibile")
           
   def toggle_connection(self):
       if self.serial_port is None:
           try:
               self.serial_port = serial.Serial(
                   port=self.port_var.get(),
                   baudrate=int(self.baud_var.get()),
                   timeout=1
               )
               self.connect_btn.config(text="Disconnetti")
               self.status_indicator.itemconfig(self.status_circle, fill='green')
               self.reading = True
               self.read_thread = threading.Thread(target=self.read_serial)
               self.read_thread.daemon = True
               self.read_thread.start()
               self.serial_port.write(b'?\n')
               self.log_message("Inviato: ?")
           except serial.SerialException as e:
               self.log_message(f"Errore: {str(e)}")
       else:
           self.reading = False
           self.serial_port.close()
           self.serial_port = None
           self.connect_btn.config(text="Connetti")
           self.status_indicator.itemconfig(self.status_circle, fill='red')
           self.log_text.config(state='normal')
           self.log_text.delete(1.0, tk.END)
           self.log_text.config(state='disabled')
           for widget in self.entries_frame.winfo_children():
               if not isinstance(widget, ttk.Frame):
                   widget.destroy()
           self.entries.clear()
           self.main_frame.update_idletasks()
           
   def read_serial(self):
       while self.reading:
           try:
               # Verifica se la porta è ancora aperta e funzionante
               if not self.serial_port or not self.serial_port.is_open or not self.serial_port.in_waiting:
                   # Prova a scrivere per verificare la connessione
                   self.serial_port.write(b'')
               
               if self.serial_port.in_waiting:
                   data = self.serial_port.readline().decode('utf-8').strip()
                   self.log_message(f"Ricevuto: {data}")
                   self.process_response(data)
               
           except (serial.SerialException, OSError) as e:
               self.log_message("Connessione persa!")
               # Aggiorna l'interfaccia nel thread principale
               self.root.after(0, self.handle_disconnect)
               break
               
           time.sleep(0.1)
           
   def handle_disconnect(self):
       """Gestisce la disconnessione imprevista"""
       self.reading = False
       if self.serial_port:
           try:
               self.serial_port.close()
           except:
               pass
       self.serial_port = None
       self.connect_btn.config(text="Connetti")
       self.status_indicator.itemconfig(self.status_circle, fill='red')
       self.log_text.config(state='normal')
       self.log_text.delete(1.0, tk.END)
       self.log_text.config(state='disabled')
       for widget in self.entries_frame.winfo_children():
           if not isinstance(widget, ttk.Frame):
               widget.destroy()
       self.entries.clear()
       self.update_ports()  # Aggiorna la lista delle porte dopo la disconnessione
       self.main_frame.update_idletasks()
           
   def send_entry(self, index):
       if self.serial_port and self.serial_port.is_open:
           text = f"{index+1}={self.entries[index].get()}\n"
           self.serial_port.write(text.encode())
           self.log_message(f"Inviato: {text}")
           
   def send_question(self):
       if self.serial_port and self.serial_port.is_open:
           self.serial_port.write(b'?\n')
           self.log_message("Inviato: ?")
       else:
           self.update_ports()
           self.log_message("Aggiornata lista porte seriali")
           
   def send_custom(self):
       if self.serial_port and self.serial_port.is_open:
           text = self.custom_entry.get() + '\n'
           self.serial_port.write(text.encode())
           self.log_message(f"Inviato custom: {text}")
           self.custom_entry.delete(0, tk.END)

if __name__ == "__main__":
   root = tk.Tk()
   app = SerialApp(root)
   root.mainloop()