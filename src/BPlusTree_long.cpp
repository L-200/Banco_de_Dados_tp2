#include "BPlusTree_long.hpp"
#include <iostream> //para debug
#include <vector> //para vetor dinâmico
#include <algorithm> //std::sort e std::find

//abrir o arquivo e incializar caso seja um arquivo novo
BPlusTree_long::BPlusTree_long(const std::string& index_file_path) {
    index_file.open(index_file_path, std::ios::in | std::ios::out | std::ios::binary);

    if(!index_file.is_open()) {
        // arquivo novo
        std::cout << "CONSTRUTOR DA ARVORE B+ (LONG): Arquivo não existe. Criando..." << std::endl;
        std::ofstream create(index_file_path, std::ios::binary);
        if(!create) { throw std::runtime_error("ERRO: Não foi possível criar o arquivo de índice"); }
        create.close();
        index_file.open(index_file_path, std::ios::in | std::ios::out | std::ios::binary);
        if(!index_file) { throw std::runtime_error("ERRO: Não foi possível abrir o arquivo de índice após criar"); }

        // inicializa metadados
        BPlusTree_long_Metadata metadata;
        metadata.root_ptr_offset = DATA_START_OFFSET_LONG; // raiz começa após metadados
        metadata.block_count = 1;

        // escreve metadados no início do arquivo
        index_file.seekp(0);
        if (!index_file.write(reinterpret_cast<const char*>(&metadata), sizeof(BPlusTree_long_Metadata))) {
            throw std::runtime_error("ERRO: Falha ao escrever metadados iniciais.");
        }

        // inicializa variáveis 
        root_ptr = metadata.root_ptr_offset;
        block_count = metadata.block_count;

        // cria e escreve o nó raiz inicial
        BPlusTree_long_Node root_node;
        root_node.is_leaf = true;
        write_block(root_ptr, root_node); // escreve o primeiro nó no DATA_START_OFFSET_LONG

        std::cout << "CONSTRUTOR DA ARVORE B+ (LONG): Arquivo criado e inicializado. root_ptr=" << root_ptr << ", block_count=" << block_count << std::endl;

    } else {
        // arquivo existente
        std::cout << "CONSTRUTOR DA ARVORE B+ (LONG): Arquivo existente aberto." << std::endl;

        // verifica tamanho mínimo para conter metadados
        index_file.seekg(0, std::ios::end);
        long file_size = index_file.tellg();

        if (file_size < sizeof(BPlusTree_long_Metadata)) {
            // arquivo existe mas é muito pequeno, deve ser tratado como novo
            std::cout << "CONSTRUTOR DA ARVORE B+ (LONG): Arquivo existente muito pequeno. Re-inicializando..." << std::endl;
            index_file.close(); // fecha para reabrir e truncar
            index_file.open(index_file_path, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
            if(!index_file) { throw std::runtime_error("ERRO: Não foi possível reabrir/truncar arquivo pequeno."); }

            BPlusTree_long_Metadata metadata;
            metadata.root_ptr_offset = DATA_START_OFFSET_LONG;
            metadata.block_count = 1;
            index_file.seekp(0);
            if (!index_file.write(reinterpret_cast<const char*>(&metadata), sizeof(BPlusTree_long_Metadata))) {
                throw std::runtime_error("ERRO: Falha ao escrever metadados iniciais (re-init).");
             }
            root_ptr = metadata.root_ptr_offset;
            block_count = metadata.block_count;
            BPlusTree_long_Node root_node;
            root_node.is_leaf = true;
            write_block(root_ptr, root_node);

        } else {
            // arquivo tem tamanho suficiente, lê metadados
            BPlusTree_long_Metadata metadata;
            index_file.seekg(0);
            if (!index_file.read(reinterpret_cast<char*>(&metadata), sizeof(BPlusTree_long_Metadata))) {
                throw std::runtime_error("ERRO: Falha ao ler metadados do arquivo existente.");
            }

            // inicializa variáveis membro com valores lidos
            root_ptr = metadata.root_ptr_offset;
            block_count = metadata.block_count;

             // validação básica
            if (root_ptr < DATA_START_OFFSET_LONG || block_count == 0 || ((unsigned long)(root_ptr + sizeof(BPlusTree_long_Node)) > (unsigned long)file_size && block_count > 0)) {
                std::cerr << "AVISO: Metadados lidos parecem invalidos! root_ptr=" << root_ptr << ", block_count=" << block_count << ", file_size=" << file_size << std::endl;
            }
        }
    }
    // Verificação final do estado do arquivo
    if (!index_file.good()) {
        std::cerr << "ERRO FATAL no Construtor BPlusTree_long: Estado do arquivo invalido apos inicializacao!" << std::endl;
        throw std::runtime_error("Estado invalido do fstream no construtor.");
    }
    std::cout<< "CONSTRUTOR DA ARVORE B+ (LONG): Arvore criada com sucesso!" <<std::endl;
}

BPlusTree_long::~BPlusTree_long() {
    if(index_file.is_open()) {
        flush_cache(); // descarrega nós modificados para o disco

        // salvando metadados atualizados
        BPlusTree_long_Metadata metadata;
        metadata.root_ptr_offset = root_ptr; // usa o valor atual da variável
        metadata.block_count = block_count; // usa o valor atual da variável

        index_file.seekp(0); // vai para o início do arquivo
        if (!index_file.write(reinterpret_cast<const char*>(&metadata), sizeof(BPlusTree_long_Metadata))) {
            std::cerr << "ERRO FATAL: Falha ao salvar metadados no destrutor!" << std::endl;

        } else {
            index_file.flush(); // garante que os metadados sejam escritos
             std::cout << "DESTRUTOR DA AROVRE B+ (LONG): Metadados salvos." << std::endl;
        }

        index_file.close();
        std::cout << "DESTRUTOR DA AROVRE B+ (LONG): Arquivo de indice fechado." << std::endl;
    }
}

long long BPlusTree_long::hash_string_to_long(const char* str) {
    std::hash<std::string> hasher;
    return static_cast<long long>(hasher(str));
}


//encontra uma chave e retorna o seu ponteiro 
f_ptr BPlusTree_long::search(long long key, int& blocks_read) {

    blocks_read = 0;

    if (block_count == 0) {
        return -1; //arvore vazia
    }

    f_ptr ptr_atual = root_ptr;
    BPlusTree_long_Node node_atual;

    while (true) {
        node_atual = read_block(ptr_atual);
        blocks_read++;

        if (node_atual.is_leaf == true) { //em um no folha procuramos pela chave exata
            for (int i = 0; i < node_atual.key_count; i++) {
                if(node_atual.keys[i] == key) {
                    return node_atual.children[i]; //retornar o ponteiro com a localização do dado
                }
            }
            return -1; //key não achada na folha
        } 
        else {
            int i = 0;
            while (i < node_atual.key_count && key >= node_atual.keys[i]) {
                i++;
            }
            ptr_atual = node_atual.children[i];
        }
    }
}

void BPlusTree_long::insert(long long key, f_ptr data_ptr) {
    long long promoted_key;
    f_ptr new_child_ptr;

    //se retornar true a chave foi promovida até a categoria de nova raiz
    if (insert_internal(root_ptr, key, data_ptr, promoted_key, new_child_ptr)) {

        BPlusTree_long_Node new_root;
        new_root.children[0] = root_ptr;
        new_root.children[1] = new_child_ptr;
        new_root.is_leaf = false;
        new_root.keys[0] = promoted_key;
        new_root.key_count = 1;
        new_root.next_leaf = -1;

        f_ptr new_root_ptr = allocate_new_block();
        write_block(new_root_ptr, new_root);
        root_ptr = new_root_ptr;
    }
}

long long BPlusTree_long::hash_string_to_long(const char* str) {
    std::hash<std::string> hasher;
    return static_cast<long long>(hasher(str));
}

long BPlusTree_long::get_total_blocks() {
    return BPlusTree_long::block_count;
}

//INICIO DAS FUNÇÕES PRIVATE

// retorna true se uma chave foi promovida, false caso contrário
// promoted_key e new_child_ptr_out são passados para ser usados em caso de retorno de valores para a promoção
bool BPlusTree_long::insert_internal(f_ptr current_ptr, long long key, f_ptr data_ptr, long long& promoted_key_out, f_ptr& new_child_ptr_out) {

    BPlusTree_long_Node current_node = read_block(current_ptr);

    if (current_node.is_leaf) { //casos base
        if (current_node.key_count < ORDER_LONG - 1) { //podemos inserir aqui
            insert_into_leaf(current_node, key, data_ptr);
            write_block(current_ptr, current_node);
            return false;
        } else { //precisamos inserir mas temos que promover alguém
            split_leaf(current_node, key, data_ptr, promoted_key_out, new_child_ptr_out);
            write_block(current_ptr, current_node);
            return true;
        }
    } else {
        int child_index = 0;
        while (child_index < current_node.key_count && key >= current_node.keys[child_index]) { //achando o child que vamos descer
            child_index++;
        }

        f_ptr child_ptr = current_node.children[child_index];

        if (insert_internal(child_ptr, key, data_ptr, promoted_key_out, new_child_ptr_out)) {
            if (current_node.key_count < ORDER_LONG - 1) {
                insert_into_internal(current_node, promoted_key_out, new_child_ptr_out);
                write_block(current_ptr, current_node);
                return false;
            } else {
                split_internal(current_node, promoted_key_out, new_child_ptr_out);
                write_block(current_ptr, current_node); //gravando lado esquerdo
                return true;
            }
        }
        return false;
    }
}

void BPlusTree_long::insert_into_leaf(BPlusTree_long_Node& leaf, long long key, f_ptr data_ptr) {
    int pos = 0;
    while (pos < leaf.key_count && leaf.keys[pos] < key) { //descobre aonde vamos enfiar
        pos++;
    }

    for (int i = leaf.key_count; i > pos; --i) { //move todo mundo pra direita (abrindo espaço)
        leaf.keys[i] = leaf.keys[i-1];
        leaf.children[i] = leaf.children[i-1];
    }

    leaf.keys[pos] = key;
    leaf.children[pos] = data_ptr;
    leaf.key_count++;
}

void BPlusTree_long::split_leaf(BPlusTree_long_Node& leaf, long long key, f_ptr data_ptr, long long& promoted_key_out, f_ptr& new_leaf_ptr_out) {
    std::vector<std::pair<long long, f_ptr>> temp_vet_pairs;
    temp_vet_pairs.reserve(ORDER_LONG);
    for (int i = 0; i < leaf.key_count; i++) {
        temp_vet_pairs.push_back({leaf.keys[i], leaf.children[i]});
    }
    temp_vet_pairs.push_back({key, data_ptr});
    std::sort(temp_vet_pairs.begin(), temp_vet_pairs.end(),[](auto &a, auto &b){ return a.first < b.first; }); 

    BPlusTree_long_Node new_leaf;
    new_leaf.is_leaf = true;
    new_leaf_ptr_out = allocate_new_block();

    int split_point = (int)temp_vet_pairs.size() / 2;
    // preenche leaf 
    leaf.key_count = 0;
    for (int i = 0; i < split_point; ++i) {
        leaf.keys[leaf.key_count] = temp_vet_pairs[i].first;
        leaf.children[leaf.key_count] = temp_vet_pairs[i].second;
        ++leaf.key_count;
    }
    // preenche new_leaf
    new_leaf.key_count = 0;
    for (int i = split_point; i < (int)temp_vet_pairs.size(); ++i) {
        new_leaf.keys[new_leaf.key_count] = temp_vet_pairs[i].first;
        new_leaf.children[new_leaf.key_count] = temp_vet_pairs[i].second;
        ++new_leaf.key_count;
    }

    new_leaf.next_leaf = leaf.next_leaf;
    leaf.next_leaf = new_leaf_ptr_out;

    promoted_key_out = new_leaf.keys[0];

    //função que chamou já vai gravar a leaf antiga
    write_block(new_leaf_ptr_out, new_leaf);
}


void BPlusTree_long::insert_into_internal(BPlusTree_long_Node& node, long long key, f_ptr child_ptr) {
    int pos = 0;
    while (pos < node.key_count && node.keys[pos] < key) {
        pos++;
    }

    for (int i = node.key_count; i > pos; --i) { //logica mudou em relação a insert_into_leaf pois os childrens dos Nodes devem ser inseridos depois das chaves
        node.keys[i] = node.keys[i-1];
        node.children[i+1] = node.children[i];
    }

    node.keys[pos] = key;
    node.children[pos+1] = child_ptr;
    node.key_count++;
}

void BPlusTree_long::split_internal(BPlusTree_long_Node& node, long long& promoted_key, f_ptr& child_ptr) {
    // copiando temporariamente as chaves e ponteiros do nó atual
    std::vector<long long> temp_vet_keys(node.keys, node.keys + node.key_count);
    std::vector<f_ptr> temp_vet_children(node.children, node.children + node.key_count + 1);

    // encontra posição de inserção da chave
    auto localizacao = std::lower_bound(temp_vet_keys.begin(), temp_vet_keys.end(), promoted_key);
    int pos = std::distance(temp_vet_keys.begin(), localizacao);

    // inserindo a nova chave e o novo filho na posição adequada
    temp_vet_keys.insert(temp_vet_keys.begin() + pos, promoted_key);
    temp_vet_children.insert(temp_vet_children.begin() + pos + 1, child_ptr); // filho sempre à direita da key


    int split_point = ORDER_LONG / 2;

    // a chave do meio é promovida para o nível superior
    promoted_key = temp_vet_keys[split_point];

    // criando o novo node
    BPlusTree_long_Node new_internal_node;
    new_internal_node.is_leaf = false;
    child_ptr = allocate_new_block();

    // ajeirando os dois nodes
    node.key_count = split_point;
    std::copy(temp_vet_keys.begin(), temp_vet_keys.begin() + split_point, node.keys);
    std::copy(temp_vet_children.begin(), temp_vet_children.begin() + split_point + 1, node.children);

    new_internal_node.key_count = static_cast<int>(temp_vet_keys.size()) - split_point - 1;
    std::copy(temp_vet_keys.begin() + split_point + 1, temp_vet_keys.end(), new_internal_node.keys);
    std::copy(temp_vet_children.begin() + split_point + 1, temp_vet_children.end(), new_internal_node.children);

    // a função que chamou já vai gravar o primeiro bloco
    write_block(child_ptr, new_internal_node);
}


BPlusTree_long_Node BPlusTree_long::read_block(f_ptr block_ptr) {
     // validação básica do ponteiro
     if (block_ptr < DATA_START_OFFSET_LONG || block_ptr % sizeof(BPlusTree_long_Node) != (DATA_START_OFFSET_LONG % sizeof(BPlusTree_long_Node))) {
          std::cerr << "ERRO FATAL: Tentativa de ler bloco em offset invalido: " << block_ptr << std::endl;
          throw std::runtime_error("Offset de leitura invalido.");
     }

    auto it = node_cache.find(block_ptr);
    if (it != node_cache.end()) { return it->second; }

    BPlusTree_long_Node node;
    index_file.seekg(block_ptr);
    if (!index_file.read(reinterpret_cast<char*>(&node), sizeof(BPlusTree_long_Node))) {
        std::cerr << "ERRO FATAL: Falha ao ler o bloco " << block_ptr << " do disco!" << std::endl;
        throw std::runtime_error("Falha na leitura do bloco do indice.");
    }

    if (node_cache.size() >= MAX_CACHE_SIZE) { flush_cache(); }
    node_cache[block_ptr] = node;
    return node;
}

void BPlusTree_long::flush_cache() {
    if (!index_file.is_open() || !index_file.good()) {return; }
    for (const auto& pair : node_cache) {
        f_ptr block_ptr = pair.first;
        const BPlusTree_long_Node& node = pair.second;
        index_file.seekp(block_ptr);
        if (!index_file.write(reinterpret_cast<const char*>(&node), sizeof(BPlusTree_long_Node))) {
            std::string error_msg = "ERRO FATAL: Falha ao escrever o bloco " + std::to_string(block_ptr) + " durante o flush_cache!";
            throw std::runtime_error(error_msg);
        }
    }
    index_file.flush();
    node_cache.clear();
}

void BPlusTree_long::write_block(f_ptr block_ptr, const BPlusTree_long_Node& node) {
     // validação básica do ponteiro
     if (block_ptr < DATA_START_OFFSET_LONG || block_ptr % sizeof(BPlusTree_long_Node) != (DATA_START_OFFSET_LONG % sizeof(BPlusTree_long_Node))) {
          std::cerr << "ERRO FATAL: Tentativa de escrever bloco em offset invalido: " << block_ptr << std::endl;
          throw std::runtime_error("Offset de escrita invalido.");
     }

    node_cache[block_ptr] = node;
    if (node_cache.size() > MAX_CACHE_SIZE) { flush_cache(); }
}

f_ptr BPlusTree_long::allocate_new_block() {
    // Flush garante que o tamanho do arquivo esteja atualizado antes de 'tellp'
    // chamar flush_cache aqui pode ser excessivo, index_file.flush() é suficiente
    index_file.flush(); // garante que escritas anteriores sejam feitas

    index_file.seekp(0, std::ios::end);
    f_ptr current_end = index_file.tellp(); // onde o arquivo termina ATUALMENTE

    // calculando onde o NOVO bloco DEVE começar
    f_ptr new_block_ptr;
    if (block_count == 0) { // situação de inicialização, embora o construtor deva cuidar disso
         new_block_ptr = DATA_START_OFFSET_LONG;
    } else {

        // o novo bloco começa no final atual, mas garantimos que está alinhado
        new_block_ptr = DATA_START_OFFSET_LONG + block_count * sizeof(BPlusTree_long_Node);
        // se o cálculo acima for diferente do final real, pode indicar corrupção
         if (new_block_ptr < current_end) {
            std::cerr << "AVISO: allocate_new_block detectou tamanho de arquivo inesperado. current_end=" << current_end << ", new_block_ptr_calc=" << new_block_ptr << std::endl;
             new_block_ptr = current_end;
             // realinhar se necessário (garante que não escrevamos em um offset "quebrado")
            if ((new_block_ptr - DATA_START_OFFSET_LONG) % sizeof(BPlusTree_long_Node) != 0) {
                new_block_ptr = DATA_START_OFFSET_LONG + ((new_block_ptr - DATA_START_OFFSET_LONG + sizeof(BPlusTree_long_Node) - 1) / sizeof(BPlusTree_long_Node)) * sizeof(BPlusTree_long_Node);
            }
         }
    }


    BPlusTree_long_Node empty_node;
    // escreve DIRETAMENTE no disco para estender o arquivo
    index_file.seekp(new_block_ptr);
    if (!index_file.write(reinterpret_cast<const char*>(&empty_node), sizeof(BPlusTree_long_Node))) {
         std::cerr << "ERRO FATAL: Falha ao alocar novo bloco " << new_block_ptr << " no disco!" << std::endl;
         throw std::runtime_error("Falha ao estender o arquivo de indice.");
    }
    index_file.flush(); // garante que a escrita foi feita

    // adiciona o nó vazio ao cache
    node_cache[new_block_ptr] = empty_node;

    block_count++; // incrementa o contador APÓS alocar com sucesso
    return new_block_ptr;
}