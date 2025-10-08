# definição de imagem base
FROM ubuntu:24.04

# instalação de dependências e remover cache do apt para manter a imagem leve
RUN apt-get update && apt-get install -y --no-install-recommends \
    g++\
    make\
    && rm -rf /var/lib/apt/lists/* 

# definir diretório de trabalho
WORKDIR /app

# copiar arquivos do projeto para o diretório de trabalho
COPY . .

# compilar o código-fonte
RUN make build

# declarando volume persistente de dados, onde vamos botar o input(antes de rodar o codigo) e output
VOLUME ["/data"]

# definição de variaveis de ambiente
ENV DATA_DIR=/data/db
ENV LOG_LEVEL=info 

# comando padrão ao iniciar o container
CMD ["/bin/bash", "-c", "echo 'Imagem construída. Use docker run para executar um dos programas: upload, findrec, seek1, seek2' && echo 'Binários disponíveis em /app/bin/:' && ls -l /app/bin"]