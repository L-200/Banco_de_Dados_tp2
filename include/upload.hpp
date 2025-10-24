#ifndef UPLOAD_HPP
#define UPLOAD_HPP

#include <string>
#include <vector>
#include "record.hpp" 

//Remove espaços em branco do início e fim da string (modifica in-place)
void trim(std::string& s);

//"limpa" a linha
bool parse_csv_line(const std::string& line, Artigo& artigo);

#endif // UPLOAD_HPP