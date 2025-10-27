# definindo o compilador
CXX = g++

# definindo flags de compilação
CXXFLAGS = -std=c++17 -Wall -Iinclude

# definindo diretórios
SRCDIR = src
INCDIR = include
BINDIR = bin

# definição de targets
TARGETS = upload findrec seek1 seek2

# arquivos fonte compartilhados entre os targets
SHARED_SRCS = $(wildcard $(SRCDIR)/BPlusTree.cpp $(SRCDIR)/BPlusTree_long.cpp $(SRCDIR)/hashing.cpp $(SRCDIR)/record.cpp)

# converte os compartilhados em arquivos objeto
SHARED_OBJS = $(SHARED_SRCS:.cpp=.o)

# nome da imagem do docker
IMAGE_NAME = tp2

# PARTE DA COMPILAÇÃO
# alvo padrão chama o build
all: build

# alvo build depende da criação de todos os executáveis
build: $(addprefix $(BINDIR)/, $(TARGETS))

# regra para criar os 4 programas
# Adicionada dependência dos headers
$(BINDIR)/%: $(SRCDIR)/%.cpp $(SHARED_OBJS) $(wildcard $(INCDIR)/*.hpp) | $(BINDIR)
	@echo "Compilando executáveis"
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# regra para gerar os arquivos objeto compartilhados
# Adicionada dependência dos headers
%.o: %.cpp $(wildcard $(INCDIR)/*.hpp)
	@echo "Compilando objetos"
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# cria o bin se ele já não existir
$(BINDIR):
	@mkdir -p $(BINDIR)

# limpando
clean:
	@echo "Limpando arquivos gerados"
	# Apaga os arquivos objeto do diretório src
	@rm -f $(SRCDIR)/*.o
	# Apaga todos os arquivos dentro de bin, exceto .gitkeep
	@find $(BINDIR) -type f -not -name '.gitkeep' -delete

# REGRAS DO DOCKER
docker-build:
	@echo "Construindo imagem Docker '$(IMAGE_NAME)'..."
	@docker build -t $(IMAGE_NAME) .

docker-run-upload:
	# Monta o diretório ./data (host) para /data (container)
	@docker run --rm -v "$(shell pwd)/data:/data" -e LOG_LEVEL=$(LOG_LEVEL) $(IMAGE_NAME) ./bin/upload /data/artigo.csv

docker-run-findrec:
	# Monta o diretório ./data (host) para /data (container) para acessar /data/db
	@docker run --rm -v "$(shell pwd)/data:/data" -e LOG_LEVEL=$(LOG_LEVEL) $(IMAGE_NAME) ./bin/findrec $(ARGS)

docker-run-seek1:
	# Monta o diretório ./data (host) para /data (container) para acessar /data/db
	@docker run --rm -v "$(shell pwd)/data:/data" -e LOG_LEVEL=$(LOG_LEVEL) $(IMAGE_NAME) ./bin/seek1 $(ARGS)

docker-run-seek2:
	# Monta o diretório ./data (host) para /data (container) para acessar /data/db
	@docker run --rm -v "$(shell pwd)/data:/data" -e LOG_LEVEL=$(LOG_LEVEL) $(IMAGE_NAME) ./bin/seek2 $(ARGS)

# regras para ajudar o usuário
# Lista todos os alvos que NÃO são arquivos
.PHONY: all build clean docker-build docker-run-upload docker-run-findrec docker-run-seek1 docker-run-seek2 help

help:
	@echo "Uso:"
	@echo "  export DATA_DIR=./data/db # Define o diretório de dados para os programas bin (obrigatório caso não use Docker)"
	@echo "  export LOG_LEVEL=<debug|info|warning|error>  # Define o nível de log para os programas Docker"
	@echo "  make build          - Compila todos os programas na pasta ./bin/"
	@echo "  make clean          - Remove todos os arquivos compilados"
	@echo "  make docker-build   - Constrói a imagem Docker"
	@echo "  make docker-run-upload - Executa o upload inicial dos dados (CSV deve estar em ./data/artigo.csv)"
	@echo "  make docker-run-findrec ARGS=<ID> - Executa o findrec com um ID"
	@echo "  make docker-run-seek1 ARGS=<ID>   - Executa o seek1 com um ID"
	@echo "  make docker-run-seek2 ARGS='<TITULO>' - Executa o seek2 com um Título"

