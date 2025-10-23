#ifndef UPLOAD_HPP
#define UPLOAD_HPP

#include <string>
#include <vector>
#include "record.hpp" 


/**
 * @brief Converte uma linha de texto do formato CSV para a estrutura Artigo
 * @param line A string contendo uma linha do arquivo CSV
 * @param artigo A referência para o objeto Artigo a ser preenchido
 * @return true se o parsing foi bem-sucedido, false caso contrário
 */
bool parse_csv_line(const std::string& line, Artigo& artigo);

#endif // UPLOAD_HPP