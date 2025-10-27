# Sobre este trabalho

Este projeto implementa programas em C++ para armazenamento e consulta de dados de artigos científicos em memória secundária, utilizando estruturas de arquivo de dados (Hashing) e índices (Árvore B+).

# Integrantes da equipe:

Lucas de Souza Cerveira Pereira

Mikaelle Costa de Santana

Roberta Graziela de Oliveira Brasil

# Pré-requisitos:

* ## Para build local (compilar diretamente na sua máquina):
    * Compilador C++ (g++ compatível com C++17)
    * GNU Make
    * Ambiente Linux

* ## Para build via docker (construir a imagem Docker com o ambiente e os programas compilados)
    * Docker instalado e rodado

* ## artigo.csv utilizado no /data
    [Clique aqui para baixá-lo](./data/artigo.csv)
# Instruções de Build

* ## Local:
    ```bash
    make build
    ```

* ## Via Docker:
    ```bash
    make docker-build
    ```

# Comandos de execução para os 4 programas

* ## Local:

    **Definindo DATA_DIR (OBRIGATÓRIO PARA TODOS OS PROGRAMAS):**
    ```bash
    export DATA_DIR=./data/db
    ```

    **Definindo Nível de Log (Opcional):**
    Antes de executar os comandos abaixo, você pode definir o nível de log no seu terminal:
    ```bash
    export LOG_LEVEL=info # Ou debug, warn, error (padrão INFO se não definido)
    ```

    **1. Carga Inicial (`upload`)**
    ```bash
    # Certifique-se que data/artigo.csv existe!
    ./bin/upload ./data/artigo.csv 
    ```

    **2. Busca Direta por ID (`findrec`)**
    ```bash
    ./bin/findrec <ID_DO_ARTIGO>

    # Exemplo:
    ./bin/findrec 1401852
    ```

    **3. Busca por ID via Índice Primário (`seek1`)**
    ```bash
    ./bin/seek1 <ID_DO_ARTIGO>

    # Exemplo:
    ./bin/seek1 1409630
    ```

    **4. Busca por Título via Índice Secundário (`seek2`)**
    ```bash
    ./bin/seek2 <TITULO_DO_ARTIGO>

    # Exemplo com Título com Espaços (não precisa de aspas aqui):
    ./bin/seek2 Gatac: A scalable and realistic testbed for multiagent decision making 
    ```

* ## Via Docker:

    **Definindo Nível de Log (Opcional):**
    ```bash
    export LOG_LEVEL=info # Ou debug, warn, error (padrão INFO se não definido)
    ```

    **1. Carga Inicial (`upload`)**
    ```bash
    # Certifique-se que data/artigo.csv existe!
    make docker-run-upload 
    ```

    **2. Busca Direta por ID (`findrec`)**
    ```bash
    make docker-run-findrec ARGS=<ID_DO_ARTIGO>

    # Exemplo:
    make docker-run-findrec ARGS=1409630 
    ```

    **3. Busca por ID via Índice Primário (`seek1`)**
    ```bash
    make docker-run-seek1 ARGS=<ID_DO_ARTIGO>

    # Exemplo:
    make docker-run-seek1 ARGS=1409630
    ```

    **4. Busca por Título via Índice Secundário (`seek2`)**
    ```bash
    # Exemplo com Título Simples:
    make docker-run-seek2 ARGS='Gatac: A scalable and realistic testbed for multiagent decision making' 

    # Exemplo com Título com Espaços (use aspas simples em volta no ARGS):
    make docker-run-seek2 ARGS='Building worlds with strokes.'
    ```

# Arquivos gerados em /data

**Todos os arquivos seguintes são gerados pelo programa upload.**

* ## data_file.dat: 
    * Descrição: O arquivo de dados principal. Armazena todos os registros Artigo completos em formato binário.
    * Organização: É uma Tabela Hash com Endereçamento Aberto (Sondagem Linear). O arquivo é pré-alocado com um tamanho fixo (750.000 blocos) para acesso rápido.

* ## primary_index.idx:
    * Descrição: O arquivo de índice primário, otimizado para buscas por ID.
    * Organização: Uma Árvore B+ (BPlusTree).
    * Chave: int ID (O ID do artigo).
    * Valor: f_ptr (O offset/ponteiro para a localização exata do registro Artigo dentro do data_file.dat).

* ## secondary_index.idx:
    * Descrição: O arquivo de índice secundário, otimizado para buscas por Título.
    * Organização: Uma Árvore B+ (BPlusTree_long) que usa chaves do tipo long long.
    * Chave: long long (O resultado de uma função de hash aplicada ao Titulo do artigo).
    * Valor: f_ptr (O offset/ponteiro para a localização exata do registro Artigo dentro do data_file.dat).

# Exemplos de entrada e saída:

## Findrec
* Entrada:
    ```bash
    make docker-run-findrec ARGS=1401787
    ```
* Saída:
    ````bash
    # Monta o diretório ./data (host) para /data (container) para acessar /data/db
    LOG_LEVEL inválido (''). Usando padrão INFO.
    [INFO]  Buscando pelo ID: 1401787
    [INFO]  
    Registro encontrado com sucesso!
    ------------------------------------------
    ID: 1401787
    Titulo: Improved regulatory oversight using real-time data monitoring technologies in the wake of Macondo
    Ano: 2014
    Autores: KM Carter, E van Oort, A Barendrecht
    Citacoes: 4
    Atualizacao: 2016
    Snippet: Improved regulatory oversight using real-time data monitoring technologies in the wake of Macondo. KM Carter, E van Oort, A Barendrecht - SPE Deepwater Drilling and  , 2014 - onepetro.org. ... in the wake of the Exxon Valdez oil spill, VTS technological advancements became ... Additionally  OCSLA mandates that OCS operations should be conducted       using technology, precautions,  and ... to require the utilization of the best available and safest technologies (BAST) on .. 
    ------------------------------------------
    [INFO]  
    --- Métricas da Busca ---
    [INFO]  Blocos lidos para encontrar o registro: 1
    [INFO]  Total de blocos no arquivo de dados: 750000
    [INFO]  Tempo de execucao do findrec: 0.360 ms
    ````
    
## Seek 1
* Entrada:
    ```bash
    make docker-run-findrec ARGS=1401775
    ```
* Saída:
    ````bash
    # Monta o diretório ./data (host) para /data (container) para acessar /data/db
    LOG_LEVEL inválido (''). Usando padrão INFO.
    [INFO]  Buscando pelo ID no indice primario: 
    [INFO]  
    Chave encontrada no indice! Ponteiro para dados: 1976183312
    [INFO]  Lendo registro do arquivo de dados...
    [INFO]  
    Registro encontrado com sucesso!
    ------------------------------------------
    ID: 1401775
    Titulo: Redefining evolutionary economics
    Ano: 2006
    Autores: M Nishibe
    Citacoes: 13
    Atualização: 2016
    Snippet: Redefining evolutionary economics. M Nishibe - Evolutionary and Institutional Economics Review, 2006 - Springer. ... limitation or locality of cognition and calculation of evolutionary agents for objects of ... as computing,  cognition, artificial intelligence, artificial life, chaos, complexity, self- organization, and autopoiesis ...  the 21st century will be the time when polymorphic systems of socioeconomy ..
    ------------------------------------------
    [INFO]  
    --- Metricas da Busca no Indice Primario ---
    [INFO]  Blocos lidos no arquivo de indice: 3
    [INFO]  Total de blocos no arquivo de indice primario: 6044
    [INFO]  Tempo de execucao do seek1: 0.799 ms
    ```` 

## Seek 2
* Entrada:
    ```bash
    make docker-run-seek2 ARGS='MLBPR: MAS for large-scale biometric pattern recognition'
    ```
* Saída:
    ````bash
    # Monta o diretório ./data (host) para /data (container) para acessar /data/db
    LOG_LEVEL inválido (''). Usando padrão INFO.
    [INFO]  Buscando pelo Titulo (truncado para 300 caracteres): "MLBPR: MAS for large-scale biometric pattern recognition"
    [INFO]  Hash encontrado no indice! Ponteiro para dados: 1999898104
    [INFO]  Lendo registro do arquivo de dados para verificacao...

    Registro encontrado com sucesso (titulo verificado)!
    ------------------------------------------
    ID: 1409597
    Titulo: MLBPR: MAS for large-scale biometric pattern recognition
    Ano: 2009
    Autores: R Meshulam, S Reches, A Yarden, S Kraus
    Citacoes: 5
    Atualizacao: 2016
    Snippet: MLBPR: MAS for large-scale biometric pattern recognition. R Meshulam, S Reches, A Yarden, S Kraus - Safety and Security in  , 2009 - Springer. Abstract Security systems can observe and hear almost anyone everywhere. However, it is  impossible to employ an adequate number of human experts to analyze the information  explosion. In this paper, we present a multi-agent framework which works in large-scale  ..
    ------------------------------------------
    [INFO]  
    --- Metricas da Busca no Indice Secundario ---
    [INFO]  Blocos lidos no arquivo de indice: 3
    [INFO]  Total de blocos no arquivo de indice secundario: 5862
    [INFO]  Tempo de execucao do seek2: 1.094 ms
    ````