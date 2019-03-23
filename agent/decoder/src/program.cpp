#include <cstdlib>
#include <iostream>
#include <stdarg.h>
#include <queue>

#include "json.h"
#include "options.h"
#include "program.h"
#include "decoder.h"

void PROGRAM::run() {
    if (!options) return bug();
    if (options->exit_flag) {
        status = EXIT_SUCCESS;
        return;
    }

    DECODER decoder(log);

    std::string input(std::istreambuf_iterator<char>(std::cin), {});
    nlohmann::json result = nlohmann::json();

    if (decoder.decode(input, &result)) {
        if (result.count("graffiti")
        &&  result.at("graffiti").is_boolean()) {
            if (result["graffiti"].get<bool>()) {
                if (dump_json(result)) {
                    status = EXIT_SUCCESS;
                }
            }
            else status = EXIT_SUCCESS;
            return;
        }
        else bug();
    }
    else {
        if (result.count("error") && result.at("error").is_string()) {
            log(get_name(), "%s", result["error"].get<std::string>().c_str());
        }
        else bug();
    }
}

bool PROGRAM::init(int argc, char **argv) {
    options = new (std::nothrow) OPTIONS(get_version(), log);
    if (!options) return false;

    if (!options->init(argc, argv)) {
        return false;
    }

    return true;
}

void PROGRAM::deinit() {
    if (options) {
        delete options;
        options = nullptr;
    }
}

int PROGRAM::get_status() const {
    return status;
}

void PROGRAM::log(const char *origin, const char *p_fmt, ...) {
    va_list ap;
    char *buf = nullptr;
    char *newbuf = nullptr;
    int buffered = 0;
    int	size = 1024;

    if (p_fmt == nullptr) return;
    buf = (char *) malloc (size * sizeof (char));

    while (1) {
        va_start(ap, p_fmt);
        buffered = vsnprintf(buf, size, p_fmt, ap);
        va_end (ap);

        if (buffered > -1 && buffered < size) break;
        if (buffered > -1) size = buffered + 1;
        else               size *= 2;

        if ((newbuf = (char *) realloc (buf, size)) == nullptr) {
            free (buf);
            return;
        } else {
            buf = newbuf;
        }
    }

    std::cerr << origin << ": " << buf << std::endl;
    free(buf);
}

void PROGRAM::bug(const char *file, int line) {
    log(get_name(), "Forbidden condition met on line %d of %s.", line, file);
}

const char *PROGRAM::get_name() const {
    return pname.c_str();
}

const char *PROGRAM::get_version() const {
    return pver.c_str();
}

bool PROGRAM::dump_json(const nlohmann::json &json, const int indent, std::string *to, const char* file, int line) {
    std::string result;

    try {
        if (indent >= 0) result = json.dump(indent);
        else             result = json.dump();
    }
    catch (nlohmann::json::type_error& e) {
        std::cerr << get_name() << ": " << e.what() << " (" << file << ":" << line << ")" << std::endl;
        return false;
    }

    if (to) to->swap(result);
    else std::cout << result << std::endl;

    return true;
}

