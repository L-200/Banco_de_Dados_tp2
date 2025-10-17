#include <BPlusTree.hpp>
#include <iostream> //para debug
#include <vector> //para vetor dinâmico
#include <algorithm> //std::sort e std::find

//abrir o arquivo e incializar caso seja um arquivo novo
BPlusTree::BPlusTree(const std::string& index_file_path) {
    index_file.open(index_file_path, std::ios::in | std::ios::out | std::ios::binary); //abre no modo binario para leitura e escrita

    //se não existir o arquivo, cria e inicializa a arvore
    
    if(!index_file.is_open()) {
        // cria o arquivo
        std::ofstream create(index_file_path, std::ios::binary);
        if(!create.is_open()) {
            throw std::runtime_error("ERRO: Não foi possível criar o arquivo de índice");
        }
        create.close();
        // reabre em modo leitura/escrita binário
        index_file.open(index_file_path, std::ios::in | std::ios::out | std::ios::binary);
        if(!index_file.is_open())
            throw std::runtime_error("ERRO: Não foi possível abrir o arquivo de índice após criar");

        //inicializando a arvore
        BPlusTreeNode node_raiz;
        node_raiz.is_leaf = true;
        root_ptr = allocate_new_block();
        write_block(root_ptr, node_raiz);
    } else {
        //se arquivo já existe, calcula o numero de blocos e assume que raiz está no bloco 0
        index_file.seekg(0, std::ios::end);
        long file_size = index_file.tellg(); //tellg fala a posição em que o cursor está

        if (file_size == 0) { //arquivo existe mas arvore ainda não foi inicializada
            BPlusTreeNode node_raiz;
            node_raiz.is_leaf = true;
            root_ptr = allocate_new_block();
            write_block(root_ptr, node_raiz);
        } else { //arvoore existe e está no inicio do arquivo
            root_ptr = 0;
            block_count = file_size / sizeof(BPlusTreeNode);
        }
    } 
}

//fecha o arquivo
BPlusTree::~BPlusTree() {
    if(index_file.is_open()) {
        index_file.close();
    }
}

//encontra uma chave e retorna o seu ponteiro 
f_ptr BPlusTree::search(int key, int& blocks_read) {

    blocks_read = 0;

    if (block_count == 0) {
        return -1; //arvore vazia
    }

    f_ptr ptr_atual = root_ptr;
    BPlusTreeNode node_atual;

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

void BPlusTree::insert(int key, f_ptr data_ptr) {
    int promoted_key;
    f_ptr new_child_ptr;

    //se retornar true a chave foi promovida até a categoria de nova raiz
    if (insert_internal(root_ptr, key, data_ptr, promoted_key, new_child_ptr)) {

        BPlusTreeNode new_root;
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

long BPlusTree::get_total_blocks() {
    return BPlusTree::block_count;
}
//INICIO DAS FUNÇÕES PRIVATE

// retorna true se uma chave foi promovida, false caso contrário
// promoted_key e new_child_ptr_out são passados para ser usados em caso de retorno de valores para a promoção
bool BPlusTree::insert_internal(f_ptr current_ptr, int key, f_ptr data_ptr, int& promoted_key_out, f_ptr& new_child_ptr_out) {

    BPlusTreeNode current_node = read_block(current_ptr);

    if (current_node.is_leaf) { //casos base
        if (current_node.key_count < ORDER - 1) { //podemos inserir aqui
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
            if (current_node.key_count < ORDER - 1) {
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

void BPlusTree::insert_into_leaf(BPlusTreeNode& leaf, int key, f_ptr data_ptr) {
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

void BPlusTree::split_leaf(BPlusTreeNode& leaf, int key, f_ptr data_ptr, int& promoted_key_out, f_ptr& new_leaf_ptr_out) {
    std::vector<std::pair<int, f_ptr>> temp_vet_pairs;
    temp_vet_pairs.reserve(ORDER);
    for (int i = 0; i < leaf.key_count; i++) {
        temp_vet_pairs.push_back({leaf.keys[i], leaf.children[i]});
    }
    temp_vet_pairs.push_back({key, data_ptr});
    std::sort(temp_vet_pairs.begin(), temp_vet_pairs.end(),[](auto &a, auto &b){ return a.first < b.first; }); 

    BPlusTreeNode new_leaf;
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


void BPlusTree::insert_into_internal(BPlusTreeNode& node, int key, f_ptr child_ptr) {
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

void BPlusTree::split_internal(BPlusTreeNode& node, int& promoted_key, f_ptr& child_ptr) {
    // copiando temporariamente as chaves e ponteiros do nó atual
    std::vector<int> temp_vet_keys(node.keys, node.keys + node.key_count);
    std::vector<f_ptr> temp_vet_children(node.children, node.children + node.key_count + 1);

    // encontra posição de inserção da chave
    auto localizacao = std::lower_bound(temp_vet_keys.begin(), temp_vet_keys.end(), promoted_key);
    int pos = std::distance(temp_vet_keys.begin(), localizacao);

    // inserindo a nova chave e o novo filho na posição adequada
    temp_vet_keys.insert(temp_vet_keys.begin() + pos, promoted_key);
    temp_vet_children.insert(temp_vet_children.begin() + pos + 1, child_ptr); // filho sempre à direita da key


    int split_point = ORDER / 2;

    // a chave do meio é promovida para o nível superior
    promoted_key = temp_vet_keys[split_point];

    // criando o novo node
    BPlusTreeNode new_internal_node;
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


BPlusTreeNode BPlusTree::read_block(f_ptr block_ptr) {
    BPlusTreeNode node;
    index_file.seekg(block_ptr);
    index_file.read(reinterpret_cast<char*>(&node), sizeof(BPlusTreeNode));
    return node;
}

void BPlusTree::write_block(f_ptr block_ptr, const BPlusTreeNode& node) {
    index_file.seekp(block_ptr);
    index_file.write(reinterpret_cast<const char*>(&node), sizeof(BPlusTreeNode));
}

f_ptr BPlusTree::allocate_new_block() {
    // move o ponteiro para o final do arquivo para encontrar o offset do novo bloco
    index_file.seekp(0, std::ios::end);
    f_ptr new_block_ptr = index_file.tellp();
    
    // escreve um nó vazio para de fato alocar o espaço no arquivo
    BPlusTreeNode empty_node{};
    write_block(new_block_ptr, empty_node);

    block_count++;
    return new_block_ptr;
}