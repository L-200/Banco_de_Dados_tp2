#include <iostream>       // Para std::cout, std::cerr, std::endl
#include <fstream>        // Para std::ifstream
#include <sstream>        // Para std::ostringstream (reconstruir título)
#include <string>         // Para std::string
#include <vector>         // (Não diretamente necessário, mas bom ter)
#include <stdexcept>      // Para std::runtime_error
#include <cstring>        // Para std::strncpy, std::strcmp, strnlen
#include <functional>     // Para std::hash
#include <chrono>
#include <iomanip> // Para stepprecision

// === Headers do projeto ===
#include "record.hpp"         // Define a struct Artigo
#include "BPlusTree_long.hpp" // Define a classe BPlusTree_long (para índice secundário)
#include "log.hpp" //para log levels


// === Função auxiliar para imprimir artigo ===
//não tem porquê de inserir log aqui, essa é a  principal funcionalidade do código !
void print_artigo(const Artigo& artigo) {
    std::cout << "------------------------------------------" << std::endl;
    std::cout << "ID: " << artigo.ID << std::endl;
    std::cout << "Titulo: ";
    // Imprime com segurança até 300 caracteres ou até o \0
    std::cout.write(artigo.Titulo, strnlen(artigo.Titulo, 300));
    std::cout << std::endl;
    std::cout << "Ano: " << artigo.Ano << std::endl;
    std::cout << "Autores: ";
    std::cout.write(artigo.Autores, strnlen(artigo.Autores, 150));
    std::cout << std::endl;
    std::cout << "Citacoes: " << artigo.Citacoes << std::endl;
    std::cout << "Atualizacao: " << artigo.Atualizacao_timestamp << std::endl; // Assumindo timestamp
    std::cout << "Snippet: ";
    // Imprime com segurança, tratando caracteres não imprimíveis se necessário
    std::cout.write(artigo.Snippet, strnlen(artigo.Snippet, 1024));
    std::cout << std::endl;
    std::cout << "------------------------------------------" << std::endl;
}

// === Função principal do programa seek2 ===
int main(int argc, char* argv[]) {
    
    auto start_time = std::chrono::high_resolution_clock::now();
    // Verificando se pelo menos um argumento (parte do título) foi passado
    if (argc < 2) {
        LOG_ERROR("Uso: " << argv[0] << " <Titulo_do_artigo>" << std::endl);
        LOG_ERROR("Dica: Se o titulo contiver espacos, não precisa de aspas." << std::endl);
        return 1;
    }

    // Reconstruindo o título completo a partir de todos os argumentos
    std::ostringstream oss;
    for (int i = 1; i < argc; ++i) {
        if (i > 1) oss << " "; // Adiciona espaço entre os argumentos
        oss << argv[i];
    }
    std::string search_titulo_completo = oss.str();

    // Truncando o título de busca
    char truncated_search_titulo[301]; // Buffer para o título truncado (igual ao struct Artigo)
    std::strncpy(truncated_search_titulo, search_titulo_completo.c_str(), 300);
    truncated_search_titulo[300] = '\0'; // Garante terminação nula

    // Definindo os caminhos absolutos para os arquivos de dados e índice
    const std::string secondary_index_path = "/data/secondary_index.idx";
    const std::string data_file_path = "/data/data_file.dat";

    LOG_INFO("Buscando pelo Titulo (truncado para 300 caracteres): \"" << truncated_search_titulo << "\"");

    try {
        // Calculando o hash DO TÍTULO TRUNCADO
        long long search_hash = BPlusTree_long::hash_string_to_long(truncated_search_titulo);
        LOG_DEBUG("Hash gerado: " << search_hash);

        // Inicializando a B+Tree secundária (deve abrir o arquivo existente)
        BPlusTree_long secondary_index(secondary_index_path);
        int blocks_read_index = 0;

        //Buscando o HASH na árvore B+
        f_ptr data_ptr = secondary_index.search(search_hash, blocks_read_index);

        bool record_found_and_verified = false; // Flag para rastrear sucesso final

        // Se o hash foi encontrado no índice
        if (data_ptr != -1) {
            LOG_INFO("Hash encontrado no indice! Ponteiro para dados: " << data_ptr);
            LOG_INFO("Lendo registro do arquivo de dados para verificacao...");

            // abre o arquivo de dados principal para ler o registro completo
            std::ifstream data_file(data_file_path, std::ios::binary);
            if (!data_file) {
                LOG_ERROR("Erro ao tentar abrir o arquivo de dados");
                throw std::runtime_error("ERRO FATAL: Não foi possivel abrir o arquivo de dados '" + data_file_path + "'");
            }

            // Posiciona no local indicado pelo índice
            data_file.seekg(data_ptr);
            if (!data_file) {
                data_file.close();
                LOG_ERROR("Falha ao posicionar cursor no arquivo de dados");
                throw std::runtime_error("ERRO FATAL: Falha ao posicionar no arquivo de dados no offset " + std::to_string(data_ptr));
            }

            Artigo found_artigo; // Cria struct para receber os dados
            // Lê o registro completo do arquivo de dados
            if (!data_file.read(reinterpret_cast<char*>(&found_artigo), sizeof(Artigo))) {
                data_file.close();
                LOG_ERROR("Falha ao ler o registro no arquivo de dados na posição do offset");
                throw std::runtime_error("ERRO FATAL: Falha ao ler o registro do arquivo de dados no offset " + std::to_string(data_ptr));
            }
            data_file.close();

            // --- VERIFICAÇÃO FINAL (Contra Colisões de Hash) ---
            // Compara o título BUSCADO (truncado) com o título LIDO DO ARQUIVO (já truncado na struct)
            if (strcmp(found_artigo.Titulo, truncated_search_titulo) == 0) {
                // Os títulos (truncados) coincidem! Encontramos o registro correto
                record_found_and_verified = true;
                std::cout << "\nRegistro encontrado com sucesso (titulo verificado)!" << std::endl;
                print_artigo(found_artigo);
            } else {
                // Hash coincidiu, mas os títulos não. É uma colisão de hash (muito rara em hash de long long)
                 std::cout << "\nAVISO: Colisao de hash detectada ou erro de dados." << std::endl;
                 std::cout << "  Hash encontrado, mas o titulo no registro não corresponde ao buscado." << std::endl;
                 std::cout << "  Titulo Buscado (truncado): " << truncated_search_titulo << std::endl;
                 std::cout << "  Titulo no Registro Lido: "; std::cout.write(found_artigo.Titulo, strnlen(found_artigo.Titulo, 300)); std::cout << std::endl;
            }

        }

        // Se o hash não foi encontrado OU se foi colisão...
        if (!record_found_and_verified) {
            LOG_INFO("\nRegistro com o titulo (truncado) \"" << truncated_search_titulo << "\" não foi encontrado.");
        }

        // Exibe as métricas de busca no índice secundário
        LOG_INFO("\n--- Metricas da Busca no Indice Secundario ---");
        LOG_INFO("Blocos lidos no arquivo de indice: " << blocks_read_index);
        try {
            LOG_INFO("Total de blocos no arquivo de indice secundario: " << secondary_index.get_total_blocks());
        } catch (...) {
            LOG_ERROR("AVISO: Não foi possivel obter o total de blocos do indice secundario.");
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = end_time - start_time;
        std::chrono::duration<double, std::milli> duration_ms_fp = duration;
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        LOG_INFO("Tempo de execucao do seek2: "
                        << std::fixed << std::setprecision(3) // mostra 3 casas decimais
                        << duration_ms_fp.count() << " ms");

    } catch (const std::runtime_error& e) {
        LOG_ERROR("ERRO FATAL durante a busca: " << e.what());
        return 1;
    } catch (...) {
        LOG_ERROR("ERRO FATAL desconhecido durante a busca." << std::endl);
        return 1;
    }

    return 0; // Terminado com sucesso
}