// log.hpp

#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>   // Para std::cout, std::cerr, std::endl
#include <string>     // Para std::string
#include <cstdlib>    // Para std::getenv()
#include <algorithm>  // Para std::transform (tornar case-insensitive)
#include <sstream>    // Para permitir streaming nos macros de log

// Definindo os Níveis de Log (enum class para segurança de tipo)
enum class LogLevel {
    ERROR = 0, // Apenas erros fatais
    WARN  = 1, // Avisos que não param a execução
    INFO  = 2, // Mensagens informativas padrão (caminhos, métricas)
    DEBUG = 3  // Mensagens detalhadas para depuração
};

// Função auxiliar para converter enum class para int (permitir comparações)
inline int logLevelValue(LogLevel l) {
    return static_cast<int>(l);
}

// Função auxiliar para remover espaços e quebras de linha
inline std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \n\r\t");
    size_t end = s.find_last_not_of(" \n\r\t");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

// Função (inline) para obter o nível de log atual
// lê a variável de ambiente LOG_LEVEL apenas uma vez e guarda o resultado
inline LogLevel getCurrentLogLevel() {
    static const LogLevel currentLevel = []() {
        LogLevel level = LogLevel::INFO; // Padrão é INFO
        const char* level_str = std::getenv("LOG_LEVEL"); // Lê variável de ambiente
        if (level_str) {
            std::string level_s = trim(level_str);        // Remove espaços e quebras de linha
            std::transform(level_s.begin(), level_s.end(), level_s.begin(), ::tolower); // Converte para minúsculas

            // Compara com os níveis conhecidos
            if (level_s == "error") level = LogLevel::ERROR;
            else if (level_s == "warning") level = LogLevel::WARN;
            else if (level_s == "info") level = LogLevel::INFO;
            else if (level_s == "debug") level = LogLevel::DEBUG;
            else {
                // Aviso se o valor for inválido
                std::cerr << "LOG_LEVEL inválido ('" << level_str << "'). Usando padrão INFO." << std::endl;
            }
        }
        return level; // Retorna o nível determinado
    }(); // Executa a lambda AGORA para inicializar currentLevel

    return currentLevel; // Retorna o nível (já calculado)
}

// macros para incluir __FILE__ e __LINE__ e permitir streaming fácil
// O 'do { ... } while(0)' é um truque comum para tornar o macro seguro em if/else
/*os ... indicam número variável de argumentos adicionais (permite o uso de <<)*/
#define LOG_MSG(level, prefix, ...) \
    do { \
        if (logLevelValue(getCurrentLogLevel()) >= logLevelValue(level)) { \
            /* Usa ostringstream para construir a mensagem permitindo '<<' */ \
            std::ostringstream oss_log_macro; \
            oss_log_macro << prefix << __VA_ARGS__; \
            /* Envia para cerr (ERRO/WARN) ou cout (INFO/DEBUG) */ \
            if (logLevelValue(level) <= logLevelValue(LogLevel::WARN)) { \
                std::cerr << oss_log_macro.str() << std::endl; \
            } else { \
                std::cout << oss_log_macro.str() << std::endl; \
            } \
        } \
    } while(0)

// Macros específicos para cada nível para facilitar o uso
#define LOG_ERROR(...) LOG_MSG(LogLevel::ERROR, "[ERROR] ", __VA_ARGS__)
#define LOG_WARN(...)  LOG_MSG(LogLevel::WARN,  "[WARN]  ", __VA_ARGS__)
#define LOG_INFO(...)  LOG_MSG(LogLevel::INFO,  "[INFO]  ", __VA_ARGS__)
#define LOG_DEBUG(...) LOG_MSG(LogLevel::DEBUG, "[DEBUG] ", __VA_ARGS__)

#endif // LOG_HPP