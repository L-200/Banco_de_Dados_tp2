#ifndef UPLOAD_HPP
#define UPLOAD_HPP

#include <string>
#include <vector>
#include "record.hpp" // Precisa da definição de struct Artigo

// Função que você irá implementar no upload.cpp
// Ela é responsável por pegar uma linha do CSV e preencher a struct Artigo.
/**
 * @brief Converte uma linha de texto do formato CSV para a estrutura Artigo.
 * @param line A string contendo uma linha do arquivo CSV.
 * @param artigo A referência para o objeto Artigo a ser preenchido.
 * @return true se o parsing foi bem-sucedido, false caso contrário.
 */
bool parse_csv_line(const std::string& line, Artigo& artigo);

// Você pode adicionar outras declarações aqui, se necessário.

#endif // UPLOAD_HPP