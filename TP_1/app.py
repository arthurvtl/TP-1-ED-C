import pandas as pd
import random

random.seed(42)

# 1. Times
times = [
    "JAVAlis",
    "ESCorpiões",
    "SemCTRL",
    "GOrilas",
    "PYthons",
    "SeQueLas",
    "NETunos",
    "LOOPardos",
    "RUSTicos",
    "REACTivos",
]

df_times = pd.DataFrame({
    "ID": range(len(times)),
    "Time": times
})
df_times.to_csv("times.csv", index=False)

# 2. Partidas
partida_colunas = ["ID", "Time1ID", "Time2ID", "GolsTime1", "GolsTime2"]

# 2.1. Campeonato Vazio
df_vazio = pd.DataFrame(columns=partida_colunas)
df_vazio.to_csv("partidas_vazio.csv", index=False)

# 2.2. Campeonato Parcial + Completo

resultados = []
for i in range(len(times)):
    for j in range(len(times)):
        if i == j:
            continue
        gols_time1 = random.randint(0, 5)
        gols_time2 = random.randint(0, 5)
        resultados.append({
            "Time1ID": i,
            "Time2ID": j,
            "GolsTime1": gols_time1,
            "GolsTime2": gols_time2
        })

random.shuffle(resultados)

# 2.2.1. Parcial (50% dos jogos)
ponto_medio = len(resultados)//2
resultados_parciais = resultados[:ponto_medio]

for i, resultado in enumerate(resultados_parciais):
    resultado["ID"] = i

df_parcial = pd.DataFrame(resultados_parciais)
df_parcial = df_parcial[partida_colunas] # Reordenar colunas
df_parcial.to_csv("partidas_parcial.csv", index=False)

# 2.2.2. Completo (100% dos jogos)
for i, resultado in enumerate(resultados):
    resultado["ID"] = i

df_completo = pd.DataFrame(resultados)
df_completo = df_completo[partida_colunas]
df_completo.to_csv("partidas_completo.csv", index=False)
