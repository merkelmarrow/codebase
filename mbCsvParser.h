#ifndef MBCSVPARSER_H
#define MBCSVPARSER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FIELD_LEN 1024
#define INITIAL_BUFFER_SIZE 4096
#define INITIAL_LINE_CAPACITY 16

// STRUCTURES

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

typedef struct CSVWriter {
    FILE *file;
    char *buffer;
    size_t buffer_len;
    size_t buffer_size;
} CSVWriter;

// FUNCTION PROTOTYPES

// EXTERNAL FUNCTIONS
CSVParser *csv_parser_create(char delimiter, bool has_header);
void csv_parser_destroy(CSVParser *parser);
bool csv_parser_parse_file(CSVParser *parser, const char *filename);
char *csv_parser_get_field(CSVParser *parser, size_t row_index,
                           size_t field_index);
char *csv_parser_get_field_by_header(CSVParser *parser, size_t row_index,
                                     const char *header);

// HELPER FUNCTIONS
static bool parse_line(ParserState *state, CSVRow *row);
static bool flush_buffer_to_file(CSVWriter *writer);
static bool add_to_buffer(CSVWriter *writer, char c);
static bool add_str_to_buffer(CSVWriter *writer, const char *str);
static bool add_field_to_buffer(CSVWriter *writer, const char *field,
                                char delimiter);
static CSVWriter *init_writer(FILE *file);
static void destroy_writer(CSVWriter *writer);
static bool write_csv_to_file(CSVWriter *writer, CSVParser *parser);

// EXTERNAL FUNC IMPLEMENTATIONS

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
        fputs("Unable to destroy CSVParser", stderr);
        return;
    }

    if (parser->headers) {
        for (size_t i = 0; i < parser->num_headers; i++) {
            free(parser->headers[i]);
        }
        free(parser->headers);
    }

    if (parser->rows) {
        for (size_t i = 0; i < parser->num_rows; i++) {
            if (parser->rows[i]) {
                for (size_t j = 0; j < parser->rows[i]->num_fields; j++) {
                    free(parser->rows[i]->fields[j]);
                }
                free(parser->rows[i]->fields);
                free(parser->rows[i]);
            }
        }
        free(parser->rows);
    }
    free(parser);
}

bool csv_parser_parse_file(CSVParser *parser, const char *filename) {
    if (!parser || !filename) {
        fputs("Invalid parser or filename\n", stderr);
        return false;
    }

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
        fputs("Failed to allocated initial buffer\n", stderr);
        return false;
    }

    size_t total_read = 0;
    size_t bytes_read;
    while ((bytes_read = fread(state.buffer + total_read, 1,
                               state.buffer_size - total_read - 1, file)) > 0) {
        total_read += bytes_read;
        if (total_read >= state.buffer_size - 1) {
            size_t new_buffer_size = state.buffer_size * 2;
            if (new_buffer_size > SIZE_MAX / 2) {
                fputs("Buffer size too large\n", stderr);
                free(state.buffer);
                fclose(file);
                return false;
            }
            char *new_buffer = realloc(state.buffer, new_buffer_size);
            if (!new_buffer) {
                fputs("Failed to reallocate buffer\n", stderr);
                free(state.buffer);
                fclose(file);
                return false;
            }
            state.buffer = new_buffer;
            state.buffer_size = new_buffer_size;
        }
    }
    state.buffer[total_read] = '\0';
    fclose(file);

    // Parse the header row if the parser expects headers
    if (parser->has_header) {
        CSVRow header_row = {NULL, 0};
        if (!parse_line(&state, &header_row)) {
            fputs("Unable to parse header line\n", stderr);
            free(state.buffer);
            return false;
        }
        parser->headers = header_row.fields;
        parser->num_headers = header_row.num_fields;
    }

    // Parse each row in the buffer
    while (state.position < total_read) {
        // Expand row storage if necessary
        if (parser->num_rows >= parser->capacity) {
            size_t new_capacity =
                parser->capacity == 0 ? INITIAL_LINE_CAPACITY : parser->capacity * 2;
            CSVRow **new_rows =
                realloc(parser->rows, new_capacity * sizeof(CSVRow *));
            if (!new_rows) {
                fputs("Failed to reallocate row storage\n", stderr);
                free(state.buffer);
                return false;
            }
            parser->rows = new_rows;
            parser->capacity = new_capacity;
        }

        // Allocate memory for the new row
        parser->rows[parser->num_rows] = malloc(sizeof(CSVRow));
        if (!parser->rows[parser->num_rows]) {
            fputs("Failed to allocate memory for CSVRow\n", stderr);
            free(state.buffer);
            return false;
        }

        // Initialize the new CSVRow
        parser->rows[parser->num_rows]->fields = NULL;
        parser->rows[parser->num_rows]->num_fields = 0;

        // Parse the line into the newly allocated row
        if (!parse_line(&state, parser->rows[parser->num_rows])) {
            fputs("Error parsing line\n", stderr);
            free(state.buffer);
            free(parser->rows[parser->num_rows]);
            parser->rows[parser->num_rows] = NULL;
            return false;
        }
        ++parser->num_rows;
    }

    // Clean up
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

// HELPER FUNC IMPLEMENTATIONS

// returns false if unsuccessful
static bool parse_line(ParserState *state, CSVRow *row) {
    char field[MAX_FIELD_LEN];
    size_t field_pos = 0;
    row->num_fields = 0;
    row->fields = NULL;

    while (state->buffer[state->position] != '\0' &&
           (state->in_quotes || state->buffer[state->position] != '\n' &&
                                                                              state->buffer[state->position] != '\r')) {
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
            // End of quoted field
            state->in_quotes = false;
            ++state->position;
            continue;
        }

        if (current == state->delimiter && !state->in_quotes) {
            // End of field, add field to row
            field[field_pos] = '\0';
            // Add field to row
            char **new_fields =
                realloc(row->fields, (row->num_fields + 1) * sizeof(char *));
            if (!new_fields) {
                fputs("Failed to allocate memory for fields\n", stderr);
                // Free previously allocated fields in case of realloc failure
                for (size_t i = 0; i < row->num_fields; i++) {
                    free(row->fields[i]);
                }
                free(row->fields);
                return false;
            }
            row->fields = new_fields;
            row->fields[row->num_fields] = strdup(field);
            if (!row->fields[row->num_fields]) {
                fputs("Failed to duplicate field string\n", stderr);
                // Free previously allocated fields and current row fields on error
                for (size_t i = 0; i < row->num_fields; i++) {
                    free(row->fields[i]);
                }
                free(row->fields);
                return false;
            }
            ++row->num_fields;
            field_pos = 0;
            ++state->position;
            continue;
        }

        if (field_pos >= MAX_FIELD_LEN - 1) {
            fputs("Field length exceeds MAX_FIELD_LEN\n", stderr);
            // Free all allocated memory if field exceeds maximum length
            for (size_t i = 0; i < row->num_fields; i++) {
                free(row->fields[i]);
            }
            free(row->fields);
            return false;
        }

        // Add character to field
        field[field_pos++] = current;
        ++state->position;
    }

    // Add the last field if there's any content or if this is not the first field
    if (field_pos > 0 || row->num_fields > 0) {
        field[field_pos] = '\0';
        char **new_fields =
            realloc(row->fields, (row->num_fields + 1) * sizeof(char *));
        if (!new_fields) {
            fputs("Failed to allocate memory for fields\n", stderr);
            for (size_t i = 0; i < row->num_fields; i++) {
                free(row->fields[i]);
            }
            free(row->fields);
            return false;
        }
        row->fields = new_fields;
        row->fields[row->num_fields] = strdup(field);
        if (!row->fields[row->num_fields]) {
            fputs("Failed to duplicate field string\n", stderr);
            for (size_t i = 0; i < row->num_fields; i++) {
                free(row->fields[i]);
            }
            free(row->fields);
            return false;
        }
        ++row->num_fields;
    }

    // Handle newline characters and advance position
    if (state->buffer[state->position] == '\r') {
        ++state->position;
    }
    if (state->buffer[state->position] == '\n') {
        ++state->position;
    }

    return true;
}

static bool flush_buffer_to_file(CSVWriter *writer) {
    if (writer->buffer_len > 0) {
        size_t bytes_written =
            fwrite(writer->buffer, 1, writer->buffer_len, writer->file);
        if (bytes_written != writer->buffer_len) {
            perror("Error writing buffer to file");
            return false;
        }
        writer->buffer_len = 0;
    }

    if (writer->buffer_size > INITIAL_BUFFER_SIZE) {
        char *new_buffer = realloc(writer->buffer, INITIAL_BUFFER_SIZE);
        if (!new_buffer) {
            perror("Failed to shrink buffer. Fatal error, exiting program. (Press "
                   "any key to continue)");
            free(writer->buffer);
            getchar();
            exit(1);
        }
        writer->buffer = new_buffer;
        writer->buffer_size = INITIAL_BUFFER_SIZE;
    }
    return true;
}

static bool add_to_buffer(CSVWriter *writer, char c) {
    if (writer->buffer_len >= writer->buffer_size) {
        if (writer->buffer_size > SIZE_MAX / 2) {
            perror("Buffer size to large, fatal error, exiting program.");
            exit(1);
        }
        writer->buffer_size *= 2;
        char *newBuffer = realloc(writer->buffer, writer->buffer_size);
        if (!newBuffer)
            return false;
        writer->buffer = newBuffer;
    }
    writer->buffer[writer->buffer_len++] = c;
    return true;
}

static bool add_str_to_buffer(CSVWriter *writer, const char *str) {
    while (*str) {
        if (!add_to_buffer(writer, *str++)) {
            return false;
        }
    }
    return true;
}

static bool add_field_to_buffer(CSVWriter *writer, const char *field,
                                char delimiter) {
    bool needs_quotes = false;
    const char *c = field;

    while (*c) {
        if (*c == '"' || *c == delimiter || *c == '\n' || *c == '\r') {
            needs_quotes = true;
            break;
        }
        ++c;
    }

    c = field;

    if (needs_quotes) {
        if (!add_to_buffer(writer, '"'))
            return false;
        while (*c) {
            if (*c == '"') {
                if (!add_str_to_buffer(writer, "\"\""))
                    return false;
            } else {
                if (!add_to_buffer(writer, *c))
                    return false;
            }
            c++;
        }
        if (!add_to_buffer(writer, '"'))
            return false;
    } else {
        if (!add_str_to_buffer(writer, field))
            return false;
    }

    return true;
}

static CSVWriter *init_writer(FILE *file) {
    CSVWriter *writer = malloc(sizeof(*writer));
    if (!writer) {
        perror("failed to initialise csv writer");
        return NULL;
    }
    writer->buffer = malloc(sizeof(char) * INITIAL_BUFFER_SIZE);
    if (!writer->buffer) {
        perror("failed to initialise csv writer buffer");
        destroy_writer(writer);
        return NULL;
    }
    writer->buffer_size = INITIAL_BUFFER_SIZE;
    writer->buffer_len = 0;
    writer->file = file;
    return writer;
}

static void destroy_writer(CSVWriter *writer) {
    free(writer->buffer);
    free(writer);
}

// TODO: ADD NULL CHECKS
static bool write_csv_to_file(CSVWriter *writer, CSVParser *parser) {
    for (size_t i = 0; i < parser->num_headers; i++) {
        if (!add_str_to_buffer(writer, parser->headers[i]))
            return false;
        if (i < parser->num_headers - 1) {
            if (!add_to_buffer(writer, parser->delimiter))
                return false;
        }
    }
    if (!add_to_buffer(writer, '\n'))
        return false;
    if (!flush_buffer_to_file(writer))
        return false;
    ;

    for (size_t i = 0; i < parser->num_rows; i++) {
        for (size_t j = 0; j < parser->rows[i]->num_fields; j++) {
            if (!add_field_to_buffer(writer, parser->rows[i]->fields[j],
                                     parser->delimiter))
                return false;
            if (j < parser->rows[i]->num_fields - 1) {
                if (!add_to_buffer(writer, parser->delimiter))
                    return false;
            }
        }
        if (!add_to_buffer(writer, '\n'))
            return false;
        if (i % 5 == 0) {
            if (!flush_buffer_to_file(writer))
                return false;
        }
    }
    if (!flush_buffer_to_file(writer))
        return false;
    return true;
}

#endif
