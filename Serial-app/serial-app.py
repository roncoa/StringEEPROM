import serial
import serial.tools.list_ports
import time
import threading
import tkinter as tk
from tkinter import ttk, scrolledtext

class SerialApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Applicazione Seriale")
        self.serial_port = None
        self.reading = False
        self.entries = []
        
        root.columnconfigure(0, weight=1)
        root.rowconfigure(0, weight=1)
        
        self.main_frame = ttk.Frame(root, padding="10")
        self.main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        self.main_frame.columnconfigure(1, weight=1)
        self.main_frame.rowconfigure(3, weight=1)  # Log text espandibile
        
        # Frame per controlli seriale (non ridimensionabile)
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
        
        # Frame per pulsanti
        btn_frame = ttk.Frame(self.main_frame)
        btn_frame.grid(row=1, column=0, columnspan=2, sticky=tk.W, pady=5)
        
        self.connect_btn = ttk.Button(btn_frame, text="Connetti", command=self.toggle_connection)
        self.connect_btn.pack(side=tk.LEFT, padx=5)
        
        self.refresh_btn = ttk.Button(btn_frame, text="Refresh", command=self.send_question)
        self.refresh_btn.pack(side=tk.LEFT, padx=5)
        
        # Frame per entries
        self.entries_frame = ttk.Frame(self.main_frame)
        self.entries_frame.grid(row=2, column=0, columnspan=2, sticky=(tk.W, tk.E))
        
        # Log (ridimensionabile)
        self.log_text = scrolledtext.ScrolledText(self.main_frame, width=40, height=10, state='disabled')
        self.log_text.grid(row=3, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), pady=5)
        
        # Frame per custom send
        send_frame = ttk.Frame(self.main_frame)
        send_frame.grid(row=4, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=5)
        send_frame.columnconfigure(1, weight=1)
        
        ttk.Label(send_frame, text="Custom:").pack(side=tk.LEFT, padx=5)
        self.custom_entry = ttk.Entry(send_frame)
        self.custom_entry.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=5)
        self.custom_entry.bind('<Return>', lambda e: self.send_custom())
        ttk.Button(send_frame, text="Invia", command=self.send_custom).pack(side=tk.LEFT, padx=5)
        
        self.update_ports()

    def log_message(self, message):
        self.log_text.config(state='normal')
        self.log_text.insert(tk.END, message + "\n")
        self.log_text.see(tk.END)
        self.log_text.config(state='disabled')

    def create_entries(self, num):
        for widget in self.entries_frame.winfo_children():
            widget.destroy()
        self.entries.clear()
        
        # Aggiungi un frame vuoto che mantiene l'altezza minima
        ttk.Frame(self.entries_frame, height=10).grid(row=0, column=0, columnspan=3, sticky=(tk.W, tk.E))
        
        for i in range(num):
            ttk.Label(self.entries_frame, text=f"{i+1}:").grid(row=i+1, column=0, padx=5, pady=2)
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
            
    def toggle_connection(self):
        if self.serial_port is None:
            try:
                self.serial_port = serial.Serial(
                    port=self.port_var.get(),
                    baudrate=int(self.baud_var.get()),
                    timeout=1
                )
                self.connect_btn.config(text="Disconnetti")
                self.reading = True
                self.read_thread = threading.Thread(target=self.read_serial)
                self.read_thread.daemon = True
                self.read_thread.start()
                
                # Invia ?
                self.serial_port.write(b'?\n')
                self.log_message("Inviato: ?")
            except serial.SerialException as e:
                self.log_message(f"Errore: {str(e)}")
        else:
            self.reading = False
            self.serial_port.close()
            self.serial_port = None
            self.connect_btn.config(text="Connetti")
            
            # Pulisci log e rimuovi widgets
            self.log_text.config(state='normal')
            self.log_text.delete(1.0, tk.END)
            self.log_text.config(state='disabled')
            
            # Rimuovi entries ma mantieni il frame vuoto
            for widget in self.entries_frame.winfo_children():
                if not isinstance(widget, ttk.Frame):
                    widget.destroy()
            self.entries.clear()
            
            self.main_frame.update_idletasks()
            
    def read_serial(self):
        while self.reading:
            if self.serial_port.in_waiting:
                data = self.serial_port.readline().decode('utf-8').strip()
                self.log_message(f"Ricevuto: {data}")
                self.process_response(data)
            time.sleep(0.1)
            
    def send_entry(self, index):
        if self.serial_port and self.serial_port.is_open:
            text = f"{index+1}={self.entries[index].get()}\n"
            self.serial_port.write(text.encode())
            self.log_message(f"Inviato: {text}")
            
    def send_question(self):
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.write(b'?\n')
            self.log_message("Inviato: ?")
            
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