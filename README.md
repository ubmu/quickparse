### quickparse

-----

Minimal and fast audio format parsing, in a single header file.

`quickparse` extracts only the metadata required for audio decoding and encoding. For example, for the Waveform Audio File Format this would be the `fmt ` and `data` chunks, as well as the `fact` chunk in non-PCM
formats.

`quickparse` was created for those who want direct access to audio metadata without a full audio library as a dependency, whether for decoding, encoding, or any other use case.
