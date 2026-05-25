import serial
import threading
from flask import Flask, render_template
from flask_socketio import SocketIO

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")

# Fixas
ADC_MAX    = 4095
TENSAO_MAX = 3.3
LIMIAR_LEVE  = 150
LIMIAR_MEDIO = 350
LIMIAR_FORTE = 600

# Configuráveis pelo frontend
config = {
    "massa_kg":    1.5,
    "fator_piezo": 50.0,
}

PORTA_BT = "COM6"  

def adc_para_newtons(valor_adc):
    tensao        = (valor_adc / ADC_MAX) * TENSAO_MAX
    aceleracao_g  = tensao * config["fator_piezo"]
    aceleracao_ms = aceleracao_g * 9.81
    return round(config["massa_kg"] * aceleracao_ms, 1)

def classificar(forca):
    if forca <= LIMIAR_LEVE:  return "LEVE"
    if forca <= LIMIAR_MEDIO: return "MEDIO"
    if forca <= LIMIAR_FORTE: return "FORTE"
    return "NOCAUTE"

def ler_bluetooth():
    try:
        bt = serial.Serial(PORTA_BT, 115200, timeout=1)
        print(f"Conectado em {PORTA_BT}")
        while True:
            linha = bt.readline().decode("utf-8").strip()
            if linha.isdigit():
                forca = adc_para_newtons(int(linha))
                intensidade = classificar(forca)
                print(f"Força: {forca}N — {intensidade}")
                socketio.emit("golpe", {
                    "forca": forca,
                    "intensidade": intensidade
                })
    except Exception as e:
        print(f"Erro Bluetooth: {e}")

@socketio.on("atualizar_config")
def atualizar_config(dados):
    for chave, valor in dados.items():
        if chave in config:
            config[chave] = float(valor)
    print(f"Config atualizada: {config}")
    socketio.emit("config_atual", config)

@socketio.on("connect")
def ao_conectar():
    socketio.emit("config_atual", config)

@app.route("/")
def index():
    return render_template("index.html")

if __name__ == "__main__":
    t = threading.Thread(target=ler_bluetooth, daemon=True)
    t.start()
    socketio.run(app, host="0.0.0.0", port=5000, debug=False)