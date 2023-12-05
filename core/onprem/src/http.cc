#ifndef __http_cc__
#define __http_cc__

#include "http.hpp"

// HTTP =====================

/**
 * @brief 
 * 
 * @param param 
 */
void noor::Http::format_value(const std::string& param) {
  auto offset = param.find_first_of("=", 0);
  auto name = param.substr(0, offset);
  auto value = param.substr((offset + 1));
  std::stringstream input(value);
  std::int32_t c;
  value.clear();

  while((c = input.get()) != EOF) {
    switch(c) {
      case '+':
        value.push_back(' ');
      break;

      case '%':
      {
        std::int8_t octalCode[3];
        octalCode[0] = (std::int8_t)input.get();
        octalCode[1] = (std::int8_t)input.get();
        octalCode[2] = 0;
        std::string octStr((const char *)octalCode, 3);
        std::int32_t ch = std::stoi(octStr, nullptr, 16);
        value.push_back(ch);
      }
      break;

      default:
        value.push_back(c);
    }
  }

  if(!value.empty() && !name.empty()) {
    add_element(name, value);
  }
}

/**
 * @brief 
 * 
 * @param in 
 */
void noor::Http::parse_uri(const std::string& in)
{
  std::string delim("\r\n");
  size_t offset = in.find(delim);

  if(std::string::npos != offset) {
    /* Qstring */
    std::string req = in.substr(0, offset);
    std::stringstream input(req);
    std::string parsed_string;
    std::string param;
    std::string value;
    bool isQsPresent = false;

    parsed_string.clear();
    param.clear();
    value.clear();

    std::int32_t c;
    while((c = input.get()) != EOF) {
      switch(c) {
        case ' ':
        {
          std::int8_t endCode[4];
          endCode[0] = (std::int8_t)input.get();
          endCode[1] = (std::int8_t)input.get();
          endCode[2] = (std::int8_t)input.get();
          endCode[3] = (std::int8_t)input.get();

          std::string p((const char *)endCode, 4);

          if(!p.compare("HTTP")) {

            if(!isQsPresent) {

              uri(parsed_string);
              parsed_string.clear();

            } else {

              value = parsed_string;
              add_element(param, value);
            }
          } else {
            /* make available to stream to be get again*/
            input.unget();
            input.unget();
            input.unget();
            input.unget();
          }

          parsed_string.clear();
          param.clear();
          value.clear();
        }
          break;

        case '+':
        {
          parsed_string.push_back(' ');
        }
          break;

        case '?':
        {
          isQsPresent = true;
          uri(parsed_string);
          parsed_string.clear();
        }
          break;

        case '&':
        {
          value = parsed_string;
          add_element(param, value);
          parsed_string.clear();
          param.clear();
          value.clear();
        }
          break;

        case '=':
        {
          param = parsed_string;
          parsed_string.clear();
        }
          break;

        case '%':
        {
          std::int8_t octalCode[3];
          octalCode[0] = (std::int8_t)input.get();
          octalCode[1] = (std::int8_t)input.get();
          octalCode[2] = 0;
          std::string octStr((const char *)octalCode, 3);
          std::int32_t ch = std::stoi(octStr, nullptr, 16);
          parsed_string.push_back(ch);
        }
          break;

        default:
        {
          parsed_string.push_back(c);
        }
          break;  
      }
    }
  }
}

void noor::Http::parse_header(const std::string& in)
{
  std::stringstream input(in);
  std::string param;
  std::string value;
  std::string parsed_string;
  std::string line_str;
  line_str.clear();

  /* getridof first request line 
   * GET/POST/PUT/DELETE <uri>?uriName[&param=value]* HTTP/1.1\r\n
   */
  std::getline(input, line_str);

  param.clear();
  value.clear();
  parsed_string.clear();

  /* iterating through the MIME Header of the form
   * Param: Value\r\n
   */
  while(!input.eof()) {
    line_str.clear();
    std::getline(input, line_str);
    std::stringstream _line(line_str);

    std::int32_t c;
    while((c = _line.get()) != EOF ) {
      switch(c) {
        case ':':
        {
          param = parsed_string;
          parsed_string.clear();
          /* getridof of first white space */
          c = _line.get();
          while((c = _line.get()) != EOF) {
            switch(c) {
              case '\r':
              case '\n':
                /* get rid of \r character */
                continue;

              default:
                parsed_string.push_back(c);
                break;
            }
          }
          /* we hit the end of line */
          value = parsed_string;
          add_element(param, value);
          parsed_string.clear();
          param.clear();
          value.clear();
        }
          break;

        default:
          parsed_string.push_back(c);
          break;
      }
    }
  }
}


std::string noor::Http::get_header(const std::string& in)
{
  std::string delimeter("\r\n\r\n");
  std::string::size_type offset = in.rfind(delimeter);
  
  if(std::string::npos != offset) {
    std::string document = in.substr(0, offset + delimeter.length());
    return(document);
  }

  return(in);
}

std::string noor::Http::get_body(const std::string& in)
{
  std::string ct = value("Content-Type");
  std::string contentLen = value("Content-Length");

  if(ct.length() && !ct.compare("application/json") && contentLen.length()) {
    auto offset = header().length() /* \r\n delimeter's length which seperator between header and body */;
    if(offset) {
      std::string document(in.substr(offset, std::stoi(contentLen)));
      return(document);
    }
  }

  return(std::string());

}

#endif /*__http_cc__*/
