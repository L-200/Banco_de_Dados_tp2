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

// Função (inline) para obter o nível de log atual
//lê a variável de ambiente LOG_LEVEL apenas uma vez e guarda o resultado
inline LogLevel getCurrentLogLevel() {
    // 'static' garante que a inicialização só ocorre na primeira chamada
    static const LogLevel currentLevel = []() {
        LogLevel level = LogLevel::INFO; // Nível padrão se não definido
        const char* level_str = std::getenv("LOG_LEVEL");
        if (level_str) {
            std::string level_s = level_str;
            std::transform(level_s.begin(), level_s.end(), level_s.begin(), ::tolower); // Case-insensitive

            if (level_s == "error") level = LogLevel::ERROR;
            else if (level_s == "warn")  level = LogLevel::WARN;
            else if (level_s == "info")  level = LogLevel::INFO;
            else if (level_s == "debug") level = LogLevel::DEBUG;
            else {
                // Imprime um aviso diretamente em cerr se o nível for inválido
                std::cerr << "[WARN] LOG_LEVEL inválido ('" << level_str << "'). Usando padrão INFO." << std::endl;
            }
        }
        return level;
    }(); // Chama a lambda imediatamente para inicializar
    
    return currentLevel;
}

// macros para incluir __FILE__ e __LINE__ e permitir streaming fácil
// O 'do { ... } while(0)' é um truque comum para tornar o macro seguro em if/else

#define LOG_MSG(level, prefix, ...) \
    do { \
        if (getCurrentLogLevel() >= level) { \
            /* Usa ostringstream para construir a mensagem permitindo '<<' */ \
            std::ostringstream oss_log_macro; \
            /* Opcional: Adiciona arquivo e linha para DEBUG */ \
            /* if (level == LogLevel::DEBUG) { oss_log_macro << "[" << __FILE__ << ":" << __LINE__ << "] "; } */ \
            oss_log_macro << prefix << __VA_ARGS__; \
            /* Envia para cerr (ERRO/WARN) ou cout (INFO/DEBUG) */ \
            if (level <= LogLevel::WARN) { \
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