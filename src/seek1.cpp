#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>
#include <cstdlib>
#include <chrono>
#include <iomanip>

#include "record.hpp"
#include "BPlusTree.hpp"
#include "log.hpp"

// Função auxiliar para imprimir os campos de um artigo
//não tem porquê de inserir log aqui, essa é a  principal funcionalidade do código !
void print_artigo(const Artigo& artigo) {
    std::cout << "------------------------------------------" << std::endl;
    std::cout << "ID: " << artigo.ID << std::endl;
    std::cout << "Titulo: " << artigo.Titulo << std::endl;
    std::cout << "Ano: " << artigo.Ano << std::endl;
    std::cout << "Autores: " << artigo.Autores << std::endl;
    std::cout << "Citacoes: " << artigo.Citacoes << std::endl;
    std::cout << "Atualização: " << artigo.Atualizacao_timestamp << std::endl; // Assumindo timestamp
    std::cout << "Snippet: " << artigo.Snippet << std::endl;
    std::cout << "------------------------------------------" << std::endl;
}

int main(int argc, char* argv[]) {
    
    auto start_time = std::chrono::high_resolution_clock::now();
    // 1. Validação dos argumentos
    if (argc != 2) {
        LOG_ERROR("Uso: " << argv[0] << " <ID_do_artigo>");
        return 1;
    }

    int search_id;
    try {
        search_id = std::stoi(argv[1]);
    } catch (const std::exception& e) {
        LOG_ERROR("ERRO: O ID informado ('" << argv[1] << "') não e um numero valido.");
        return 1;
    }

    const std::string primary_index_path = "/data/primary_index.idx";
    const std::string data_file_path = "/data/data_file.dat";

    LOG_INFO("Buscando pelo ID no indice primario: ");

    try {
        // 2. Inicializa o índice (que agora deve ABRIR o arquivo existente)
        BPlusTree primary_index(primary_index_path);
        int blocks_read_index = 0;

        // 3. Executa a busca no índice
        f_ptr data_ptr = primary_index.search(search_id, blocks_read_index);

        // 4. Verifica o resultado
        if (data_ptr != -1) {
            LOG_INFO("\nChave encontrada no indice! Ponteiro para dados: " << data_ptr);
            LOG_INFO("Lendo registro do arquivo de dados...");

            // Abre o arquivo de dados para ler o registro
            std::ifstream data_file(data_file_path, std::ios::binary);
            if (!data_file) {

                LOG_ERROR("ERRO FALTAR: Não foi possivel abrir o arquivo de dados '" << data_file_path << "' para leitura.");
                throw std::runtime_error("Falha ao abrir arquivo de dados.");
            }

            // Posiciona no local exato
            data_file.seekg(data_ptr);
            if (!data_file) { // Verifica se seekg falhou
                data_file.close();
                LOG_ERROR("ERRO FATAL: Falha ao posicionar no arquivo de dados no offset " << data_ptr);
                throw std::runtime_error("Falha no seekg do arquivo de dados.");
            }


            Artigo found_artigo;
            // Lê o registro
            if (!data_file.read(reinterpret_cast<char*>(&found_artigo), sizeof(Artigo))) {
                data_file.close();
                LOG_ERROR("ERRO FATAL: Falha ao ler o registro do arquivo de dados no offset " << data_ptr);
                LOG_ERROR("  -> Verifique se o data_ptr esta correto e se o arquivo de dados não esta corrompido.");
                throw std::runtime_error("Falha na leitura do arquivo de dados.");
            }
            data_file.close();

            LOG_INFO("\nRegistro encontrado com sucesso!");
            print_artigo(found_artigo);
        } else {
            LOG_INFO("\nRegistro com ID " << search_id << " não foi encontrado no indice.");
        }

        // 5. Exibe as métricas
        LOG_INFO("\n--- Metricas da Busca no Indice Primario ---");
        LOG_INFO("Blocos lidos no arquivo de indice: " << blocks_read_index);
        // Adicionado try-catch em volta de get_total_blocks para segurança
        try {
            LOG_INFO("Total de blocos no arquivo de indice primario: " << primary_index.get_total_blocks());
        } catch (...) {
            LOG_ERROR("AVISO: não foi possivel obter o total de blocos do indice.");
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = end_time - start_time;
        std::chrono::duration<double, std::milli> duration_ms_fp = duration;
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        LOG_INFO("Tempo de execucao do seek1: "
                        << std::fixed << std::setprecision(3) // mostra 3 casas decimais
                        << duration_ms_fp.count() << " ms");


    } catch (const std::runtime_error& e) {
        LOG_ERROR("ERRO FATAL durante a busca: " << e.what());
        return 1;
    } catch (...) {
        LOG_ERROR("ERRO FATAL desconhecido durante a busca.");
        return 1;
    }

    return 0;
}