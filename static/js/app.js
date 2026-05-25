const socket = io();

const cores = {
  LEVE:    "#4fc3f7",
  MEDIO:   "#ffd600",
  FORTE:   "#ff6b00",
  NOCAUTE: "#ff2d2d",
};

const intervalos = {
  LEVE:    "0 - 149 N",
  MEDIO:   "150 - 349 N",
  FORTE:   "350 - 599 N",
  NOCAUTE: "600+ N",
};

let total = 0, somaForca = 0, maxForca = 0;

socket.on("config_atual", (cfg) => {
  document.getElementById("cfg-massa").value = cfg.massa_kg;
  document.getElementById("cfg-fator").value = cfg.fator_piezo;
});

socket.on("golpe", (dados) => {
  total++;
  somaForca += dados.forca;
  if (dados.forca > maxForca) maxForca = dados.forca;

  document.getElementById("forca").textContent       = dados.forca;
  document.getElementById("forca").style.color       = cores[dados.intensidade];
  document.getElementById("intensidade").textContent = dados.intensidade;
  document.getElementById("intervalo").textContent   = intervalos[dados.intensidade];

  document.getElementById("st-total").textContent = total;
  document.getElementById("st-max").textContent   = maxForca;
  document.getElementById("st-media").textContent = Math.round(somaForca / total);
});

function salvarConfig() {
  socket.emit("atualizar_config", {
    massa_kg:    document.getElementById("cfg-massa").value,
    fator_piezo: document.getElementById("cfg-fator").value,
  });

  const st = document.getElementById("status-config");
  st.textContent = "Configuração aplicada!";
  st.classList.add("ok");
  setTimeout(() => { st.textContent = ""; st.classList.remove("ok"); }, 2500);
}