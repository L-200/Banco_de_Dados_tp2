//COMANDO PARA USO: g++ -std=c++17 -Iinclude src/BPlusTree.cpp tests/test_bplus_tree.cpp -o test_bplus_tree
//NÃO SE ESQUEÇA DE MUDAR O const int ORDER = 340 PARA const int ORDER = 4

#include <iostream>
#include <cassert> // Para usar a função assert()
#include <cstdio>  // Para usar a função remove()
#include <vector>  // Para testes mais complexos se necessário

#include "BPlusTree.hpp" // Inclui a classe BPlusTree COM CACHE

int main() {
    // IMPORTANTE: Mude ORDER em BPlusTree.hpp para um valor baixo (ex: 4 ou 5) para estes testes!
    const std::string test_file = "test_tree_cached.idx"; // Nome diferente para evitar conflito

    std::cout << "--- Iniciando testes da BPlusTree com Cache ---" << std::endl;
    std::cout << "AVISO: Certifique-se de que ORDER em BPlusTree.hpp esta baixo (ex: 4 ou 5)!" << std::endl;

    // Limpa o arquivo de testes anteriores
    remove(test_file.c_str());

    // --- Teste 1: Inserção Simples e Busca Imediata (Cache Hit Provável) ---
    std::cout << "  [TESTE 1] Insercao simples e busca imediata..." << std::endl;
    { // Bloco para controlar o tempo de vida da 'tree1'
        BPlusTree tree1(test_file);
        int blocks_read = 0;

        tree1.insert(10, 1000);
        tree1.insert(20, 2000);

        // Busca imediatamente após inserir (deve usar o cache se implementado)
        assert(tree1.search(10, blocks_read) == 1000);
        assert(tree1.search(20, blocks_read) == 2000);
        assert(tree1.search(99, blocks_read) == -1); // Chave inexistente

        std::cout << "  ---> Insercoes e buscas imediatas OK." << std::endl;
        // tree1 é destruída aqui, chamando flush_cache()
    }
    std::cout << "  [PASSOU TESTE 1]" << std::endl;


    // --- Teste 2: Persistência (Verifica se flush_cache funcionou) ---
    std::cout << "  [TESTE 2] Verificacao de persistencia apos flush..." << std::endl;
    { // Bloco para controlar o tempo de vida da 'tree2'
        BPlusTree tree2(test_file); // Cria NOVO objeto, forçando leitura do DISCO
        int blocks_read = 0;

        // Verifica se os dados inseridos no Teste 1 ainda existem após recarregar
        assert(tree2.search(10, blocks_read) == 1000);
        assert(tree2.search(20, blocks_read) == 2000);
        assert(tree2.search(99, blocks_read) == -1);

        std::cout << "  ---> Dados persistiram no disco apos flush." << std::endl;
        // tree2 é destruída aqui
    }
     std::cout << "  [PASSOU TESTE 2]" << std::endl;


    // --- Teste 3: Split de Folha e Persistência ---
     std::cout << "  [TESTE 3] Split de folha e persistencia..." << std::endl;
    {
        BPlusTree tree3(test_file); // Reabre com dados dos testes anteriores
        int blocks_read = 0;

        // Supondo ORDER = 4 (máx 3 chaves por nó)
        tree3.insert(30, 3000); // Nó folha agora tem {10, 20, 30}
        assert(tree3.search(30, blocks_read) == 3000);

        tree3.insert(5, 500);  // Deve causar split da folha {10, 20, 30}

        // Verifica buscas imediatas
        assert(tree3.search(5, blocks_read) == 500);
        assert(tree3.search(10, blocks_read) == 1000);
        assert(tree3.search(20, blocks_read) == 2000);
        assert(tree3.search(30, blocks_read) == 3000);
        std::cout << "  ---> Split de folha e buscas imediatas OK." << std::endl;
        // tree3 é destruída, flush_cache() chamado
    }
    { // Abre novamente para verificar persistência do split
        BPlusTree tree4(test_file);
        int blocks_read = 0;
        std::cout << "  ---> Verificando persistencia do split de folha..." << std::endl;
        assert(tree4.search(5, blocks_read) == 500);
        assert(tree4.search(10, blocks_read) == 1000);
        assert(tree4.search(20, blocks_read) == 2000);
        assert(tree4.search(30, blocks_read) == 3000);
        std::cout << "  ---> Split de folha persistiu no disco." << std::endl;
    }
    std::cout << "  [PASSOU TESTE 3]" << std::endl;

    // --- Teste 4: Split da Raiz e Persistência ---
    std::cout << "  [TESTE 4] Split da raiz e persistencia..." << std::endl;
     {
        BPlusTree tree5(test_file); // Reabre
        int blocks_read = 0;

        // Continua inserindo (com ORDER=4) para forçar split da raiz
        tree5.insert(15, 1500);
        tree5.insert(25, 2500); // Pode causar split interno
        tree5.insert(35, 3500); // Deve causar split da raiz

        // Verifica buscas imediatas
        assert(tree5.search(15, blocks_read) == 1500);
        assert(tree5.search(25, blocks_read) == 2500);
        assert(tree5.search(35, blocks_read) == 3500);
        assert(tree5.search(5, blocks_read) == 500); // Chave antiga
        std::cout << "  ---> Split da raiz e buscas imediatas OK." << std::endl;
     }
     { // Abre novamente para verificar persistência
         BPlusTree tree6(test_file);
         int blocks_read = 0;
         std::cout << "  ---> Verificando persistencia do split da raiz..." << std::endl;
         assert(tree6.search(15, blocks_read) == 1500);
         assert(tree6.search(25, blocks_read) == 2500);
         assert(tree6.search(35, blocks_read) == 3500);
         assert(tree6.search(5, blocks_read) == 500);
         std::cout << "  ---> Split da raiz persistiu no disco." << std::endl;
     }
    std::cout << "  [PASSOU TESTE 4]" << std::endl;


    // --- Limpeza Final ---
    remove(test_file.c_str());
    std::cout << "--- Todos os testes da BPlusTree com Cache passaram! ---" << std::endl;
    std::cout << "IMPORTANTE: Reverta ORDER em BPlusTree.hpp para o valor alto calculado!" << std::endl;

    return 0;
}