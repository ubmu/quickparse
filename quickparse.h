#ifndef QUICKPARSE_H
#define QUICKPARSE_H

#include <stdint.h>
#include <stdio.h>

#define XK_SENTINEL_SIZE                0

#define XK_CONTAINER_IDENTIFIER_RIFF    0x74636166

#define XK_FORM_TYPE_IDENTIFIER_WAVE    0x45564157

#define XK_CHUNK_IDENTIFIER_FMT         0x20746d66
#define XK_CHUNK_IDENTIFIER_DATA        0x61746164
#define XK_CHUNK_IDENTIFIER_FACT        0x74636166

typedef enum {
    XK_SUCCESS,
    XK_INVALID_CHUNK,
} XK_STATUS;

/* The audio format to be parsed quickly. */
typedef enum {
    XK_FORMAT_WAVE,
} XK_FORMAT;

typedef struct {
    uint16_t format_tag;
    uint16_t channel_count;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
} _default_format_chunk;

typedef struct {
    long long offset;
    uint32_t size;
    uint64_t size_64; /* Used when size field is set to -1, indicating the real size is 64-bit stored in 'ds64'. */
} _default_data_chunk;

typedef struct {
    _default_format_chunk format;
    _default_data_chunk data;
    XK_STATUS status;
} _audio_chunks;

typedef struct {
    uint32_t identifier;
    uint32_t size;
} _default_chunk_header;

_default_chunk_header _read_chunk_header(FILE *stream);
_default_format_chunk _parse_default_format_chunk(FILE *stream);

/* Parses only the `fmt` and `data` chunks of a RIFF-WAVE file, returning the
 * format metadata and the offset of the data payload within the stream.
 *
 * The caller is responsible for reading the data payload using the returned
 * offset and size. See BLAH for a utility function that does this.
 *
 * Requires the following guarantees:
 *  - The file is uncompressed PCM.
 *  - The file follows the order: RIFF header -> `fmt` chunk -> `data` chunk.
 *
 * There WILL be errors if the above guarantees are not met. For files that
 * cannot be guaranteed to follow this layout, use `xk_read_wave` instead.
 *
 * This function is faster than its counterpart `xk_read_wave` as there is no
 * support for compressed or extended variants, and no chunk traversal is
 * required as the metadata is read directly at known offsets.
 */
_audio_chunks xk_read_wave_fast(FILE *stream);

#ifdef QUICKPARSE_IMPL

_default_chunk_header _read_chunk_header(FILE *stream) {
    _default_chunk_header header;
    fread(&header, sizeof(header), 1, stream);
    return header;
}

_default_format_chunk _parse_default_format_chunk(FILE *stream) {
    _default_format_chunk format_chunk;
    fread(&format_chunk.format_tag,      sizeof(uint16_t), 1, stream);
    fread(&format_chunk.channel_count,   sizeof(uint16_t), 1, stream);
    fread(&format_chunk.sample_rate,     sizeof(uint32_t), 1, stream);
    fread(&format_chunk.byte_rate,       sizeof(uint32_t), 1, stream);
    fread(&format_chunk.block_align,     sizeof(uint16_t), 1, stream);
    fread(&format_chunk.bits_per_sample, sizeof(uint16_t), 1, stream);
    return format_chunk;
}

_audio_chunks xk_read_wave_fast(FILE *stream) {
    static const uint8_t _DEFAULT_FMT_OFFSET = 12;
    static const uint8_t _DEFAULT_FMT_SIZE = 16;
    static const uint8_t _DEFAULT_DATA_OFFSET = 36;
    _audio_chunks audio_chunks = {0};

    /* Skip the RIFF master chunk. */
    fseek(stream, _DEFAULT_FMT_OFFSET, SEEK_SET);
    _default_chunk_header format_header = _read_chunk_header(stream);
    if (format_header.identifier != XK_CHUNK_IDENTIFIER_FMT || format_header.size != 16) {
        /* The identifier or size did not match expected guarantee. */
        audio_chunks.status = XK_INVALID_CHUNK;
        return audio_chunks;
    }

    _default_format_chunk format_chunk = _parse_default_format_chunk(stream);
    audio_chunks.format = format_chunk;

    _default_chunk_header data_header = _read_chunk_header(stream);
    /* 64-bit check on 'data' size, as it is one of few chunks that can realistically be 64-bit. */
    if (data_header.identifier != XK_CHUNK_IDENTIFIER_DATA) {
        audio_chunks.status = XK_INVALID_CHUNK;
        return audio_chunks;
    }

    _default_data_chunk data_chunk = {_DEFAULT_DATA_OFFSET, data_header.size, XK_SENTINEL_SIZE};
    audio_chunks.data = data_chunk;
    return audio_chunks;
}

//
//_audio_chunks xk_regular_read(FILE *stream, XK_FORMAT format) {
//    _audio_chunks audio_chunks;
//    switch ( format ) {
//        case: XK_FORMAT_WAVE:
//            xk_read_wave( stream, &audio_chunks );
//            break;
//
//        default:
//            return NULL;
//    }
//
//    return audio_chunks;
//}

#endif /* QUICKPARSE_IMPL */
#endif /* QUICKPARSE_H */
