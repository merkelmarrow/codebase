#ifndef MBCSVPARSER_H
#define MBCSVPARSER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FIELD_LEN 1024
#define INITIAL_BUFFER_SIZE 1024
#define INITIAL_LINE_CAPACITY 16

typedef struct CSVRow {
    char **fields;
    size_t num_fields;
} CSVRow;

typedef struct CSVParser {
    CSVRow **rows;
    size_t num_rows;
    size_t capacity;
    char delimiter;
    bool has_header;
    char **headers;
    size_t num_headers;
} CSVParser;

typedef struct ParserState {
    char *buffer;
    size_t buffer_size;
    size_t position;
    bool in_quotes;
    char delimiter;
} ParserState;

// returns false if unsuccessful
static bool parse_line(ParserState *state, CSVRow *row) {
    char field[MAX_FIELD_LEN];
    size_t field_pos = 0;
    row->num_fields = 0;
    row->fields = NULL;

    while (state->buffer[state->position] != '\0' &&
           state->buffer[state->position] != '\n') {
        char current = state->buffer[state->position];

        if (current == '"' && !state->in_quotes) {
            state->in_quotes = true;
            ++state->position;
            continue;
        }

        if (current == '"' && state->in_quotes) {
            // handle escaped quotes
            if (state->buffer[state->position + 1] == '"') {
                field[field_pos++] = '"';
                state->position += 2;
                continue;
            }
            // else
            state->in_quotes = false;
            ++state->position;
            continue;
        }

        if (current == state->delimiter && !state->in_quotes) {
            field[field_pos] = '\0';
            // Add field to row
            char **new_fields =
                realloc(row->fields, (row->num_fields + 1) * sizeof(char *));
            if (!new_fields) {
                perror("Failed to realloc fields");
                return false;
            }
            row->fields = new_fields;
            row->fields[row->num_fields] = strdup(field);
            if (!row->fields[row->num_fields]) {
                perror("Failed to strdup field");
                return false;
            }
            ++row->num_fields;
            field_pos = 0;
            ++state->position;
            continue;
        }

        field[field_pos++] = current;
        ++state->position;

        if (field_pos >= MAX_FIELD_LEN - 1) {
            perror("Field is too long");
            return false;
        }
    }

    // Add last field
    if (field_pos > 0 || row->num_fields > 0) {
        field[field_pos] = '\0';
        char **new_fields =
            realloc(row->fields, (row->num_fields + 1) * sizeof(char *));
        if (!new_fields) {
            perror("Failed to realloc fields");
            return false;
        }
        row->fields = new_fields;
        row->fields[row->num_fields] = strdup(field);
        if (!row->fields[row->num_fields]) {
            perror("Failed to strdup field");
            return false;
        }
        ++row->num_fields;
    }

    // Skip newline
    if (state->buffer[state->position] == '\n') {
        ++state->position;
    }

    return true;
}

CSVParser *csv_parser_create(char delimiter, bool has_header) {
    CSVParser *parser = malloc(sizeof(CSVParser));
    if (!parser) {
        perror("Failed to create CSVParser");
        return NULL;
    }

    parser->rows = NULL;
    parser->num_rows = 0;
    parser->capacity = 0;
    parser->delimiter = delimiter;
    parser->has_header = has_header;
    parser->headers = NULL;
    parser->num_headers = 0;

    return parser;
}

void csv_parser_destroy(CSVParser *parser) {
    if (!parser) {
        perror("Unable to destroy CSVParser");
        return;
    }

    if (parser->headers) {
        for (size_t i = 0; i < parser->num_headers; i++) {
            free(parser->headers[i]);
        }
        free(parser->headers);
    }

    for (size_t i = 0; i < parser->num_rows; i++) {
        for (size_t j = 0; j < parser->rows[i]->num_fields; j++) {
            free(parser->rows[i]->fields[j]);
        }
        free(parser->rows[i]->fields);
        free(parser->rows[i]);
    }
    free(parser->rows);
    free(parser);
}

bool csv_parser_parse_file(CSVParser *parser, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Unable to open file");
        return false;
    }

    ParserState state = {.buffer = malloc(INITIAL_BUFFER_SIZE * sizeof(char)),
                         .buffer_size = INITIAL_BUFFER_SIZE,
                         .position = 0,
                         .in_quotes = false,
                         .delimiter = parser->delimiter};

    if (!state.buffer) {
        fclose(file);
        perror("Buffer malloc failed");
        return false;
    }

    size_t total_read = 0;
    size_t bytes_read;
    while ((bytes_read = fread(state.buffer + total_read, 1,
                               state.buffer_size - total_read - 1, file)) > 0) {
        total_read += bytes_read;
        if (total_read >= state.buffer_size - 1) {
            state.buffer_size *= 2;
            char *new_buffer = realloc(state.buffer, state.buffer_size);
            if (!new_buffer) {
                perror("new_buffer realloc fail");
                free(state.buffer);
                fclose(file);
                return false;
            }
            state.buffer = new_buffer;
        }
    }
    state.buffer[total_read] = '\0';
    fclose(file);

    if (parser->has_header) {
        CSVRow header_row = {NULL, 0};
        if (!parse_line(&state, &header_row)) {
            perror("Unable to parse header line");
            free(state.buffer);
            if (header_row.fields) {
                for (size_t i = 0; i < header_row.num_fields; i++) {
                    free(header_row.fields[i]);
                }
                free(header_row.fields);
            }
            return false;
        }
        parser->headers = header_row.fields;
        parser->num_headers = header_row.num_fields;
    }

    while (state.position < total_read) {
        if (parser->num_rows >= parser->capacity) {
            size_t new_capacity =
                parser->capacity == 0 ? INITIAL_LINE_CAPACITY : parser->capacity * 2;
            CSVRow **new_rows =
                realloc(parser->rows, new_capacity * sizeof(CSVRow *));
            if (!new_rows) {
                free(state.buffer);
                perror("Error during realloc *new_rows");
                return false;
            }
            parser->rows = new_rows;
            parser->capacity = new_capacity;
        }
        parser->rows[parser->num_rows] = malloc(sizeof(CSVRow));
        if (!parser->rows[parser->num_rows]) {
            free(state.buffer);
            perror("Failed to allocate memory for CSVRow");
            return false;
        }
        if (!parse_line(&state, parser->rows[parser->num_rows])) {
            free(state.buffer);
            perror("Error in parse_line step in parse_file");
            return false;
        }
        ++parser->num_rows;
    }

    free(state.buffer);
    return true;
}

char *csv_parser_get_field(CSVParser *parser, size_t row_index,
                           size_t field_index) {
    if (row_index >= parser->num_rows)
        return NULL;
    if (field_index >= parser->rows[row_index]->num_fields)
        return NULL;
    return parser->rows[row_index]->fields[field_index];
}

char *csv_parser_get_field_by_header(CSVParser *parser, size_t row_index,
                                     const char *header) {
    if (!parser->has_header)
        return NULL;

    size_t header_index = 0;
    for (; header_index < parser->num_headers; header_index++) {
        if (strcmp(parser->headers[header_index], header) == 0)
            break;
    }
    if (header_index >= parser->num_headers)
        return NULL;
    return csv_parser_get_field(parser, row_index, header_index);
}



#endif
