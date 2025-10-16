//COMANDO PARA USO: g++ -std=c++17 -Iinclude src/BPlusTree.cpp tests/test_bplus_tree.cpp -o test_bplus_tree
//NÃO SE ESQUEÇA DE MUDAR O const int ORDER = 340 PARA const int ORDER = 4

#include <string.h>
#include <iostream>
#include <cassert> // Para usar a função assert()
#include <cstdio>  // Para usar a função remove()

#include "BPlusTree.hpp" // Inclui a classe que queremos testar

// Função principal do nosso programa de teste
int main() {
    const std::string test_file = "test_tree.idx";

    // --- SETUP: Garante um ambiente limpo para o teste ---
    remove(test_file.c_str()); // Deleta o arquivo de índice de testes anteriores

    // --- INÍCIO DOS TESTES ---
    std::cout << "Iniciando testes da BPlusTree..." << std::endl;

    // Teste 1: Inserção simples e busca
    {
        std::cout << "  [TESTE 1] Insercao simples e busca..." << std::endl;
        BPlusTree tree(test_file);
        int blocks_read = 0;

        // inserimos a chave 10, que aponta para o dado na posição 1000
        tree.insert(10, 1000);
        // inserimos a chave 20, que aponta para o dado na posição 2000
        tree.insert(20, 2000);

        // verificação (assert para o programa se a busca falhar)
        assert(tree.search(10, blocks_read) == 1000);
        assert(tree.search(20, blocks_read) == 2000);
        assert(tree.search(99, blocks_read) == -1); // testa uma chave que não existe

        std::cout << "  [PASSOU]" << std::endl;
    } // o objeto tree é destruído aqui, fechando o arquivo

    // Teste 2: Inserção causando divisão de nó folha
    {
        std::cout << "  [TESTE 2] Insercao causando split de folha..." << std::endl;
        BPlusTree tree(test_file); // reabre o arquivo com os dados do teste anterior
        int blocks_read = 0;

        tree.insert(30, 3000);
        // a chave '5' vai causar a divisão da folha (que já tem 10, 20, 30)
        tree.insert(5, 500);

        // verifica se todas as chaves ainda podem ser encontradas após o split
        assert(tree.search(10, blocks_read) == 1000);
        assert(tree.search(20, blocks_read) == 2000);
        assert(tree.search(30, blocks_read) == 3000);
        assert(tree.search(5, blocks_read) == 500);

        std::cout << "  [PASSOU]" << std::endl;
    }

    // Teste 3: Inserção causando divisão da raiz (aumento da altura)
    {
        std::cout << "  [TESTE 3] Insercao causando split da raiz..." << std::endl;
        BPlusTree tree(test_file);
        int blocks_read = 0;

        // com ORDER=4, inserir mais algumas chaves deve forçar a raiz a se dividir
        tree.insert(15, 1500);
        tree.insert(25, 2500);
        tree.insert(35, 3500); // esta inserção deve causar o split da raiz

        // verifica se todas as chaves continuam acessíveis
        assert(tree.search(15, blocks_read) == 1500);
        assert(tree.search(25, blocks_read) == 2500);
        assert(tree.search(35, blocks_read) == 3500);
        assert(tree.search(10, blocks_read) == 1000); // verifica uma chave antiga

        std::cout << "  [PASSOU]" << std::endl;
    }

    // TEARDOWN: Limpa o arquivo de teste 
    remove(test_file.c_str());
    std::cout << "Todos os testes passaram!" << std::endl;

    return 0;
}