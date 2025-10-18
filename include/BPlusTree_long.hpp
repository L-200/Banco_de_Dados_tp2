#ifndef BPlusTree_long_HPP
#define BPlusTree_long_HPP

#include <string>
#include <vector>
#include <fstream>

//sizeof(is_leaf) + sizeof(key_count) + sizeof(keys) + sizeof(children) + sizeof(next_leaf) <= 4096
//1 + 4 + (8 * (m - 1)) + (8 * m) + 8 <= 4096
//m <= 255.68
const int ORDER_LONG = 255;

// long para representar os ponteiros para outros blocos no arquivo
using f_ptr = long; //-1 para nulo

// layout de um unico nó da B+ tree
struct BPlusTree_long_Node {
    bool is_leaf;                 //flag para indicar se o nó é uma folha
    int key_count;                //número de chaves atualmente no nó
    long long keys[ORDER_LONG - 1];          //array para armazenar as chaves (ex: IDs dos artigos), precisa do hashing para mapear o id do artigo para a chave
    f_ptr children[ORDER_LONG];        //array de ponteiros para os nós filhos
    
    f_ptr next_leaf; // ponteiro para o próximo nó folha
    // (para nós folha, os ponteiros podem apontar para os registros no arquivo de dados ou para um próximo nó folha)

    //construtor para inicializar um nó vazio
    BPlusTree_long_Node() : is_leaf(false), key_count(0), next_leaf(-1) {
        for (int i = 0; i < ORDER_LONG - 1; ++i) keys[i] = 0; // inicializa o array de chaves com zeros
        for (int i = 0; i < ORDER_LONG; ++i) children[i] = -1; // inicializa o array de filhos com ponteiros nulos
    }
};

// gerencia o arquivo de índice e as operações de alto nível
class BPlusTree_long {
public:
    // abre/cria o arquivo de índice
    BPlusTree_long(const std::string& index_file_path);
    
    // fecha o arquivo "~" 
    ~BPlusTree_long();

    // função principal para inserir uma chave e o ponteiro para o registro de dados
    void insert(long long key, f_ptr data_ptr);

    // função principal para buscar uma chave, retornando o ponteiro para o registro de dados e o numero de blocos lidos
    f_ptr search(long long key, int& blocks_read);

    long get_total_blocks();

private:
    std::fstream index_file;    // gerencia conexão para ler e escrever no arquivo de índice
    f_ptr root_ptr;             // ponteiro para o nó raiz no arquivo
    long block_count;           // contador total de blocos no arquivo

    // lê um bloco do arquivo de índice e o carrega em uma struct de nó
    BPlusTree_long_Node read_block(f_ptr block_ptr);

    // escreve o conteúdo de uma struct de nó em um bloco específico do arquivo
    void write_block(f_ptr block_ptr, const BPlusTree_long_Node& node);
    
    // aloca um novo bloco no final do arquivo e retorna seu ponteiro
    f_ptr allocate_new_block();

    // função auxiliar de insert_internal para inserir em uma folha 
    void insert_into_leaf(BPlusTree_long_Node& leaf, long long key, f_ptr data_ptr);

    // função auxiliar de insert_internal para separar uma folha
    void split_leaf(BPlusTree_long_Node& leaf, long long key, f_ptr data_ptr, long long& promoted_key_out, f_ptr& new_leaf_ptr_out);

    //função axuiliar de insert_internal para inserir um valor
    void insert_into_internal(BPlusTree_long_Node& node, long long key, f_ptr child_ptr);

    // função auxiliar de insert_internal para separar um nó interno
    void split_internal(BPlusTree_long_Node& node, long long& key_in_out, f_ptr& child_in_out);

    // função auxiliar recursiva para a inserção
    bool insert_internal(f_ptr current_ptr, long long key, f_ptr data_ptr, long long& promoted_key_out, f_ptr& new_child_ptr_out);

    // função para lidar com a divisão de um nó que está cheio
    void split_node(f_ptr parent_ptr, int child_index, f_ptr child_ptr);
};

#endif // BPlusTree_long_HPP