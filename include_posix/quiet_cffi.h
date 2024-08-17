/**
 * Representation for single sample containing sound
 */
typedef float quiet_sample_t;

typedef unsigned long long time_t;

typedef enum {
    quiet_success,
    quiet_mem_fail,
    quiet_encoder_bad_config,
    quiet_profile_malformed_json,
    quiet_profile_missing_key,
    quiet_profile_invalid_profile,
    quiet_msg_size,
    quiet_would_block,
    quiet_timedout,
    quiet_io,
} quiet_error;

/**
 * Get last error set by libquiet
 *
 * quiet_get_last_error retrieves the last error set. If
 * libquiet was compiled with pthread, then this error will be specific
 * to the thread where this is called.
 */
quiet_error quiet_get_last_error();

/**
 * DC-blocking filter options
 *
 * This DC blocker is applied near the end of the signal chain so that any
 * leftover DC component is removed. This is important for audio signals as we
 * do not want to send any DC out to speakers
 *
 * transfer function H(z)=(1 - (z^-1))/(1 - (1-alpha)*(z^-1))
 */
typedef struct { float alpha; } quiet_dc_filter_options;

/**
 * Resampler options
 *
 * Controls arbitrary resample unit used by libquiet after generating 44.1kHz
 * signal or before decoding signal
 *
 * This resampler will be applied to set the sample rate to the rate given
 * when creating an encoder or decoder
 */
typedef struct {
    // filter bank delay
    size_t delay;

    // filter passband bandwidth
    float bandwidth;

    // filter sidelobe suppression
    float attenuation;

    // filter bank size
    size_t filter_bank_size;
} quiet_resampler_options;

/**
 * Modulator options
 *
 * This set of options is used only by the encoder
 *
 * The modulator is a combination element which interpolates the encoded signal
 * (rescaling in frequency domain) and then mixes it onto a carrier of a given
 * frequency. Finally, a gain is applied, and an optional DC blocker removes DC
 * components.
 */
typedef struct {
    /**
     * Numerical value for shape of interpolation filter
     *
     * These values correspond to those used by liquid DSP. In particular,
     *
     * 1: Nyquist Kaiser
     *
     * 2: Parks-McClellan
     *
     * 3: Raised Cosine
     *
     * 4: Flipped Exponential (Nyquist)
     *
     * 5: Flipped Hyperbolic Secant (Nyquist)
     *
     * 6: Flipped Arc-Hyperbolic Secant (Nyquist)
     *
     * 7: Root-Nyquist Kaiser (Approximate Optimum)
     *
     * 8: Root-Nyquist Kaiser (True Optimum)
     *
     * 9: Root Raised Cosine
     *
     * 10: Harris-Moerder-3
     *
     * 11: GMSK Transmit
     *
     * 12: GMSK Receive
     *
     * 13: Flipped Exponential (root-Nyquist)
     *
     * 14: Flipped Hyperbolic Secant (root-Nyquist)
     *
     * 15: Flipped Arc-Hyperbolic Secant (root-Nyquist)
     *
     * All other values invalid
     */
    unsigned int shape;

    /// interpolation factor
    unsigned int samples_per_symbol;

    /// interpolation filter delay
    unsigned int symbol_delay;

    /// interpolation roll-off factor
    float excess_bw;

    /// carrier frequency, [0, 2*pi)
    float center_rads;

    /// gain, [0, 0.5]
    float gain;

    /// dc blocker options
    quiet_dc_filter_options dc_filter_opt;
} quiet_modulator_options;

/**
 * Demodulator options
 *
 * This set of options is used only by the decoder
 *
 * The demodulator is a combination element which inverts the operations of the
 * modulator. It first mixes down from the carrier and then performs decimation
 * to recover the signal.
 */
typedef struct {
    /**
     * Numerical value for shape of decimation filter
     *
     * This uses the same set of values as quiet_modulator_options.shape
     */
    unsigned int shape;

    /// decimation factor
    unsigned int samples_per_symbol;

    /// decimation filter delay
    unsigned int symbol_delay;

    /// decimation roll-off factor
    float excess_bw;

    /// carrier frequency, [0, 2*pi)
    float center_rads;
} quiet_demodulator_options;

typedef enum quiet_checksum_schemes {
    /* These values correspond to those used by liquid DSP */
    /// no error-detection
    quiet_checksum_none = 1,
    /// 8-bit checksum
    quiet_checksum_8bit,
    /// 8-bit CRC
    quiet_checksum_crc8,
    /// 16-bit CRC
    quiet_checksum_crc16,
    /// 24-bit CRC
    quiet_checksum_crc24,
    /// 32-bit CRC
    quiet_checksum_crc32,
} quiet_checksum_scheme_t;

typedef enum quiet_error_correction_schemes {
    /* These values correspond to those used by liquid DSP */
    /// no error-correction
    quiet_error_correction_none = 1,
    /// simple repeat code, r1/3
    quiet_error_correction_repeat_3,
    /// simple repeat code, r1/5
    quiet_error_correction_repeat_5,
    /// Hamming (7,4) block code, r1/2 (really 4/7)
    quiet_error_correction_hamming_7_4,
    /// Hamming (7,4) with extra parity bit, r1/2
    quiet_error_correction_hamming_7_4_parity,
    /// Hamming (12,8) block code, r2/3
    quiet_error_correction_hamming_12_8,
    /// Golay (24,12) block code, r1/2
    quiet_error_correction_golay_24_12,
    /// SEC-DED (22,16) block code, r8/11
    quiet_error_correction_secded_22_16,
    /// SEC-DED (39,32) block code
    quiet_error_correction_secded_39_32,
    /// SEC-DED (72,64) block code, r8/9
    quiet_error_correction_secded_72_64,
    /// convolutional code r1/2, K=7, dfree=10
    quiet_error_correction_conv_12_7,
    /// convolutional code r1/2, K=9, dfree=12
    quiet_error_correction_conv_12_9,
    /// convolutional code r1/3, K=9, dfree=18
    quiet_error_correction_conv_13_9,
    /// convolutional code 1/6, K=15, dfree<=57 (Heller 1968)
    quiet_error_correction_conv_16_15,
    /// perforated convolutional code r2/3, K=7, dfree=6
    quiet_error_correction_conv_perf_23_7,
    /// perforated convolutional code r3/4, K=7, dfree=5
    quiet_error_correction_conv_perf_34_7,
    /// perforated convolutional code r4/5, K=7, dfree=4
    quiet_error_correction_conv_perf_45_7,
    /// perforated convolutional code r5/6, K=7, dfree=4
    quiet_error_correction_conv_perf_56_7,
    /// perforated convolutional code r6/7, K=7, dfree=3
    quiet_error_correction_conv_perf_67_7,
    /// perforated convolutional code r7/8, K=7, dfree=3
    quiet_error_correction_conv_perf_78_7,
    /// perforated convolutional code r2/3, K=9, dfree=7
    quiet_error_correction_conv_perf_23_9,
    /// perforated convolutional code r3/4, K=9, dfree=6
    quiet_error_correction_conv_perf_34_9,
    /// perforated convolutional code r4/5, K=9, dfree=5
    quiet_error_correction_conv_perf_45_9,
    /// perforated convolutional code r5/6, K=9, dfree=5
    quiet_error_correction_conv_perf_56_9,
    /// perforated convolutional code r6/7, K=9, dfree=4
    quiet_error_correction_conv_perf_67_9,
    /// perforated convolutional code r7/8, K=9, dfree=4
    quiet_error_correction_conv_perf_78_9,
    /// Reed-Solomon m=8, n=255, k=223
    quiet_error_correction_reed_solomon_223_255
} quiet_error_correction_scheme_t;

typedef enum quiet_modulation_schemes {
    /* These values correspond to those used by liquid DSP */
    /// phase-shift keying-2
    quiet_modulation_psk2 = 1,
    /// phase-shift keying-4
    quiet_modulation_psk4,
    /// phase-shift keying-8
    quiet_modulation_psk8,
    /// phase-shift keying-16
    quiet_modulation_psk16,
    /// phase-shift keying-32
    quiet_modulation_psk32,
    /// phase-shift keying-64
    quiet_modulation_psk64,
    /// phase-shift keying-128
    quiet_modulation_psk128,
    /// phase-shift keying-256
    quiet_modulation_psk256,

    /// differential phase-shift keying-2
    quiet_modulation_dpsk2,
    /// differential phase-shift keying-4
    quiet_modulation_dpsk4,
    /// differential phase-shift keying-8
    quiet_modulation_dpsk8,
    /// differential phase-shift keying-16
    quiet_modulation_dpsk16,
    /// differential phase-shift keying-32
    quiet_modulation_dpsk32,
    /// differential phase-shift keying-64
    quiet_modulation_dpsk64,
    /// differential phase-shift keying-128
    quiet_modulation_dpsk128,
    /// differential phase-shift keying-256
    quiet_modulation_dpsk256,

    /// amplitude-shift keying-2
    quiet_modulation_ask2,
    /// amplitude-shift keying-4
    quiet_modulation_ask4,
    /// amplitude-shift keying-8
    quiet_modulation_ask8,
    /// amplitude-shift keying-16
    quiet_modulation_ask16,
    /// amplitude-shift keying-32
    quiet_modulation_ask32,
    /// amplitude-shift keying-64
    quiet_modulation_ask64,
    /// amplitude-shift keying-128
    quiet_modulation_ask128,
    /// amplitude-shift keying-256
    quiet_modulation_ask256,

    /// quadrature amplitude-shift keying-4
    quiet_modulation_qask4,
    /// quadrature amplitude-shift keying-8
    quiet_modulation_qask8,
    /// quadrature amplitude-shift keying-16
    quiet_modulation_qask16,
    /// quadrature amplitude-shift keying-32
    quiet_modulation_qask32,
    /// quadrature amplitude-shift keying-64
    quiet_modulation_qask64,
    /// quadrature amplitude-shift keying-128
    quiet_modulation_qask128,
    /// quadrature amplitude-shift keying-256
    quiet_modulation_qask256,
    /// quadrature amplitude-shift keying-512
    quiet_modulation_qask512,
    /// quadrature amplitude-shift keying-1024
    quiet_modulation_qask1024,
    /// quadrature amplitude-shift keying-2048
    quiet_modulation_qask2048,
    /// quadrature amplitude-shift keying-4096
    quiet_modulation_qask4096,
    /// quadrature amplitude-shift keying-8192
    quiet_modulation_qask8192,
    /// quadrature amplitude-shift keying-16384
    quiet_modulation_qask16384,
    /// quadrature amplitude-shift keying-32768
    quiet_modulation_qask32768,
    /// quadrature amplitude-shift keying-65536
    quiet_modulation_qask65536,

    /// amplitude phase-shift keying-4
    quiet_modulation_apsk4,
    /// amplitude phase-shift keying-8
    quiet_modulation_apsk8,
    /// amplitude phase-shift keying-16
    quiet_modulation_apsk16,
    /// amplitude phase-shift keying-32
    quiet_modulation_apsk32,
    /// amplitude phase-shift keying-64
    quiet_modulation_apsk64,
    /// amplitude phase-shift keying-128
    quiet_modulation_apsk128,
    /// amplitude phase-shift keying-256
    quiet_modulation_apsk256,

    /// binary phase-shift keying
    quiet_modulation_bpsk,

    /// quaternary phase-shift keying
    quiet_modulation_qpsk,

    /// on-off keying
    quiet_modulation_ook,

    /// square quadrature amplitude-shift keying-32
    quiet_modulation_sqask32,

    /// square quadrature amplitude-shift keying-128
    quiet_modulation_sqask128,

    /// V.29 star constellation
    quiet_modulation_v29,

    /// optimal quadrature amplitude-shift keying-16
    quiet_modulation_opt_qask16,
    /// optimal quadrature amplitude-shift keying-32
    quiet_modulation_opt_qask32,
    /// optimal quadrature amplitude-shift keying-64
    quiet_modulation_opt_qask64,
    /// optimal quadrature amplitude-shift keying-128
    quiet_modulation_opt_qask128,
    /// optimal quadrature amplitude-shift keying-256
    quiet_modulation_opt_qask256,

    /// Virginia Tech logo constellation
    quiet_modulation_vtech,
} quiet_modulation_scheme_t;

/**
 * Encoder options for OFDM
 *
 * These options configure the behavior of OFDM, orthogonal frequency division
 * multiplexing, as used by the encoder. OFDM places the modulated symbols on
 * to multiple orthogonal subcarriers. This can help the decoder estabilish
 * good equalization when used on a system with uneven filtering.
 */
typedef struct {
    /// total number of subcarriers used, inlcuding guard bands and pilots
    unsigned int num_subcarriers;

    /// number of cyclic prefix samples between symbols
    unsigned int cyclic_prefix_len;

    /// number of taper window between symbols
    unsigned int taper_len;

    /// number of extra guard subcarriers inserted on left (low freq)
    size_t left_band;

    /// number of extra guard subcarriers inserted on right (high freq)
    size_t right_band;
} quiet_ofdm_options;

/**
 * @enum quiet_encoding_t
 * Encoder mode
 *
 * Selects operational mode for encoder/decoder. OFDM and Modem mode
 * use the same modulation schemes while gmsk ignores the supplied
 * scheme and uses its own
 */
typedef enum encodings {
    /// Encode/decode in OFDM mode
    ofdm_encoding,

    /// Encode/decode in modem mode
    modem_encoding,

    /**
     * Encode/decode in gaussian minimum shift keying mode
     *
     * GMSK mode does not offer the modulation modes given by the other
     * encodings. It has a fairly limited bitrate, but the advantage of
     * GMSK is that its receiver does not need to compute any FFTs, making
     * it suitable for low-power receivers or situations with little
     * computational capacity.
     */
    gmsk_encoding,
} quiet_encoding_t;

/**
 * Encoder options
 *
 * This specifies a complete set of options for the encoder in libquiet.
 */
typedef struct {
    /// OFDM options, used only by OFDM mode
    quiet_ofdm_options ofdmopt;

    /// Interpolation filter and carrier frequency options
    quiet_modulator_options modopt;

    /// Resampler configuration (if specified frequency is not 44.1kHz)
    quiet_resampler_options resampler;

    /// Encoder mode, one of {ofdm_encoding, modem_encoding, gmsk_encoding}
    quiet_encoding_t encoding;

    quiet_checksum_scheme_t checksum_scheme;
    quiet_error_correction_scheme_t inner_fec_scheme;
    quiet_error_correction_scheme_t outer_fec_scheme;
    quiet_modulation_scheme_t mod_scheme;

    /**
     * Header schemes
     * These control the frame header properties
     * Only used if header_override_defaults = true
     */
    bool header_override_defaults;
    quiet_checksum_scheme_t header_checksum_scheme;
    quiet_error_correction_scheme_t header_inner_fec_scheme;
    quiet_error_correction_scheme_t header_outer_fec_scheme;
    quiet_modulation_scheme_t header_mod_scheme;

    /**
     * Maximum frame length
     *
     * This value controls the maximum length of the user-controlled
     * section of the frame. There is overhead in starting new frames,
     * and each frame performs its own CRC check which either accepts or
     * rejects the frame. A frame begins with a synchronization section
     * which the decoder uses to detect and lock on to the frame. Over time,
     * the synchronization will drift, which makes shorter frames easier to
     * decode than longer frames.
     */
    size_t frame_len;
} quiet_encoder_options;

/**
 * Decoder options
 *
 * This specifies a complete set of options for the decoder in libquiet.
 *
 * In order for a decoder to decode the signals from an encoder, certain
 * options must match between both. In particular, the encoding mode and
 * modopt/demodopt must match. Additionally, if ofdm_encoding is used,
 * then the ofdmopt must also match. If the header options are overriden
 * in the encoder, then they must also be overriden in the decoder.
 */
typedef struct {
    /// OFDM options, used only by OFDM mode
    quiet_ofdm_options ofdmopt;

    /// Decimation filter and carrier frequency options
    quiet_demodulator_options demodopt;

    /// Resampler configuration (if specified frequency is not 44.1kHz)
    quiet_resampler_options resampler;

    /// Encoder mode, one of {ofdm_encoding, modem_encoding, gmsk_encoding}
    quiet_encoding_t encoding;

    /**
     * Header schemes
     * These control the frame header properties
     * Only used if header_override_defaults = true
     */
    bool header_override_defaults;
    quiet_checksum_scheme_t header_checksum_scheme;
    quiet_error_correction_scheme_t header_inner_fec_scheme;
    quiet_error_correction_scheme_t header_outer_fec_scheme;
    quiet_modulation_scheme_t header_mod_scheme;

    /**
     * Enable debug mode on receiver
     *
     * In order for this flag to work, libquiet must be compiled in debug mode
     * (`#define QUIET_DEBUG 1`). Once enabled, this mode causes the decoder to
     * use liquid to create debug files which can be viewed in matlab/octave.
     * These files have the filename format framesync_d, where d is an
     * increasing number. These files can be useful for tracking the decoder's
     * behavior.
     */
    bool is_debug;
} quiet_decoder_options;

/**
 * Complex value
 */
typedef struct {
    float real;
    float imaginary;
} quiet_complex;

/**
 * Decoder frame stats
 *
 * This contains information about the decoding process related to a
 * single frame. The frame may have been detected but failed to
 * pass checksum or may have been successfully received.
 */
typedef struct {
    /// Raw symbols, in complex plane, as seen after decimation and downmixing
    const quiet_complex *symbols;
    size_t num_symbols;

    /// Magnitude of vector from received symbols to reference symbols, in dB
    float error_vector_magnitude;

    /// Power level of received signal after decimation and downmixing, in dB
    float received_signal_strength_indicator;

    bool checksum_passed;
} quiet_decoder_frame_stats;

/**
 * Get decoder profile from file
 * @param f file pointer which contains a valid JSON libquiet profile set
 * @param profilename the string key of the profile to fetch
 *
 * libquiet's configuration options are fairly numerous, and testing can be
 * frustrating when configuration requires recompilation. For this reason,
 * libquiet provides a JSON file containing multiple sets of configuration
 * -- profiles -- and functions to read and validate them.
 *
 * Each profile provides access to every option contained in
 * quiet_encoder_options/quiet_decoder_options. It is hoped that this will
 * give good default options and provide a starting place for users to
 * tune new profiles.
 *
 * quiet_decoder_profile_file reads the profile given by profilename
 * from the file pointer and returns the corresponding quiet_decoder_options.
 *
 * @return a pointer to an initialized quiet_decoder_options or NULL if
 *  decoding failed. must be freed by caller (with free()).
 */
quiet_decoder_options *quiet_decoder_profile_file(FILE *f,
                                                  const char *profilename);

/**
 * Get decoder profile from filename
 * @param fname path to a file which will be opened and read, must contain a
 *  valid JSON liquiet profile set
 * @param profilename the string key of the profile to fetch
 *
 * quiet_decoder_profile_filename reads the profile given by profilename
 * from the file located at filename and returns the corresponding
 * quiet_decoder_options.
 *
 * @return a pointer to an initialized quiet_decoder_options or NULL if
 *  decoding failed. must be freed by caller (with free()).
 */
quiet_decoder_options *quiet_decoder_profile_filename(const char *fname,
                                                      const char *profilename);

/**
 * Get decoder profile from string
 * @param input a string containing a valid JSON libquiet profile set
 * @param profilename the string key of the profile to fetch
 *
 * quiet_decoder_profile_str reads the profile given by profilename from
 * the input and returns the corresponding quiet_decoder_options.
 *
 * @return a pointer to an initialized quiet_decoder_options or NULL if
 *  decoding failed. must be freed by caller (with free()).
 */
quiet_decoder_options *quiet_decoder_profile_str(const char *input,
                                                 const char *profilename);

/**
 * Get encoder profile from file
 * @param f file pointer which contains a valid JSON libquiet profile set
 * @param profilename the string key of the profile to fetch
 *
 * quiet_encoder_profile_file reads the profile given by profilename
 * from the file pointer and returns the corresponding quiet_encoder_options.
 *
 * @return a pointer to an initialized quiet_encoder_options or NULL if
 *  decoding failed. must be freed by caller (with free()).
 */
quiet_encoder_options *quiet_encoder_profile_file(FILE *f,
                                                  const char *profilename);

/**
 * Get encoder profile from filename
 * @param fname path to a file which will be opened and read, must contain a
 *  valid JSON liquiet profile set
 * @param profilename the string key of the profile to fetch
 *
 * quiet_encoder_profile_filename reads the profile given by profilename
 * from the file located at filename and returns the corresponding
 * quiet_encoder_options.
 *
 * @return a pointer to an initialized quiet_encoder_options or NULL if
 *  decoding failed. must be freed by caller (with free()).
 */
quiet_encoder_options *quiet_encoder_profile_filename(const char *fname,
                                                      const char *profilename);

/**
 * Get encoder profile from string
 * @param input a string containing a valid JSON libquiet profile set
 * @param profilename the string key of the profile to fetch
 *
 * quiet_encoder_profile_str reads the profile given by profilename from
 * the input and returns the corresponding quiet_encoder_options.
 *
 * @return a pointer to an initialized quiet_encoder_options or NULL if
 *  decoding failed. must be freed by caller (with free()).
 */
quiet_encoder_options *quiet_encoder_profile_str(const char *input,
                                                 const char *profilename);

/**
 * Get list of profile keys from file
 * @param f file pointer which contains a valid JSON file
 * @param numkeys return value for number of keys found
 *
 * quiet_profile_keys_file reads the JSON file and fetches the keys from the
 * top-level dictionary. It does not perform validation on the profiles
 * themselves, which could be invalid.
 *
 * @return an array of strings with key names or NULL if JSON parsing failed.
 *  must be freed by caller (with free()). the length of this array will be
 *  written to numkeys.
 */
char **quiet_profile_keys_file(FILE *f, size_t *numkeys);

/**
 * Get list of profile keys from filename
 * @param fname path to a file which will be opened and read, must contain a
 *  valid JSON file
 * @param numkeys return value for number of keys found
 *
 * quiet_profile_keys_filename reads the JSON file found at fname and fetches
 * the keys from the top-level dictionary. It does not perform validation on
 * the profiles themselves, which could be invalid.
 *
 * @return an array of strings with key names or NULL if JSON parsing failed.
 *  must be freed by caller (with free()). the length of this array will be
 *  written to numkeys.
 */
char **quiet_profile_keys_filename(const char *fname, size_t *numkeys);

/**
 * Get list of profile keys from string
 * @param input a string containing a valid JSON file
 * @param numkeys return value for number of keys found
 *
 * quiet_profile_keys_str reads the JSON in input and fetches the keys from the
 * top-level dictionary. It does not perform validation on the profiles
 * themselves, which could be invalid.
 *
 * @return an array of strings with key names or NULL if JSON parsing failed.
 *  must be freed by caller (with free()). the length of this array will be
 *  written to numkeys.
 */
char **quiet_profile_keys_str(const char *input, size_t *numkeys);

/**
 * @struct quiet_encoder
 * Sound encoder
 */
struct quiet_encoder;
typedef struct quiet_encoder quiet_encoder;

/**
 * Create encoder
 * @param opt quiet_encoder_options containing encoder configuration
 * @param sample_rate Sample rate that encoder will generate at
 *
 * quiet_encoder_create creates and initializes a new libquiet encoder for a
 * given set of options and sample rate. As libquiet makes use of its own
 * resampler, it is suggested to use the default sample rate of your device,
 * so as to not invoke any implicit resamplers.
 *
 * @return pointer to a new encoder object, or NULL if creation failed
 */
quiet_encoder *quiet_encoder_create(const quiet_encoder_options *opt, float sample_rate);

/**
 * Send a single frame
 * @param e encoder object
 * @param buf user buffer containing the frame payload
 * @param len the number of bytes in buf
 *
 * quiet_encoder_send copies the frame provided by the user to an internal
 * transmit queue. By default, this is a nonblocking call and will fail if
 * the queue is full. However, if quiet_encoder_set_blocking has been
 * called first, then it will wait for as much as the timeout length
 * specified there if the frame cannot be immediately written.
 *
 * The frame provided must be no longer than the maximum frame length of the
 * encoder. If the frame is longer, it will be rejected entirely, and no data
 * will be transmitted.
 *
 * If libquiet was built and linked with pthread, then this function may be
 * called from any thread, and by multiple threads concurrently.
 *
 * quiet_encoder_send will return 0 if the queue is closed to signal EOF.
 *
 * quiet_encoder_send will return a negative value and set the last error
 * to quiet_timedout if the send queue is full and no space was made before
 * the timeout
 *
 * quiet_encoder_send will return a negative value and set the last error
 * to quiet_would_block if the send queue is full and the encoder is in
 * nonblocking mode
 *
 * @return the number of bytes copied from the buffer, 0 if the queue
 * is closed, or -1 if sending failed
 */
ssize_t quiet_encoder_send(quiet_encoder *e, const void *buf, size_t len);

/**
 * Set blocking mode of quiet_encoder_send
 * @param e encoder object
 * @param sec time_t number of seconds to block for
 * @param nano long number of nanoseconds to block for
 *
 * quiet_encoder_set_blocking changes the behavior of quiet_encoder_send so
 * that it will block until a frame can be written. It will block for
 * approximately (nano + 1000000000*sec) nanoseconds.
 *
 * If `sec` and `nano` are both 0, then quiet_encoder_send will block
 * indefinitely until a frame is sent.
 *
 * This function is only supported on systems with pthread. Calling
 * quiet_encoder_set_blocking on a host without pthread will assert
 * false.
 *
 */
void quiet_encoder_set_blocking(quiet_encoder *e, time_t sec, long nano);

/**
 * Set nonblocking mode of quiet_encoder_send
 * @param e encoder object
 *
 * quiet_encoder_set_nonblocking changes the behavior of quiet_encoder_send
 * so that it will not block if it cannot write a frame. This function
 * restores the default behavior after quiet_encoder_set_blocking has
 * been called.
 *
 */
void quiet_encoder_set_nonblocking(quiet_encoder *e);

/**
 * Set blocking mode of quiet_encoder_emit
 * @param e encoder object
 * @param sec time_t number of seconds to block for
 * @param nano long number of nanoseconds to block for
 *
 * quiet_encoder_set_emit_blocking changes quiet_encoder_emit so that it will block
 * until a frame is read. It will block for approximately (nano + 1000000000*sec)
 * nanoseconds.
 *
 * quiet_encoder_emit may emit some empty (silence) samples if one frame is available
 * but more frames are needed for the full length of the block given to quiet_encoder_emit.
 * That is, quiet_encoder_emit will not block the tail of one frame while waiting for the next.
 *
 * If `sec` and `nano` are both 0, then quiet_encoder_emit will block
 * indefinitely until a frame is read.
 *
 * This function is only supported on systems with pthread. Calling
 * quiet_encoder_set_blocking on a host without pthread will assert
 * false.
 */
//void quiet_encoder_set_emit_blocking(quiet_encoder *e, time_t sec, long nano);

/**
 * Set nonblocking mode of quiet_encoder_emit
 * @param e encoder object
 *
 * quiet_encoder_set_emit_nonblocking changes the behavior of quiet_encoder_emit
 * so that it will not block if it cannot read a frame. This function
 * restores the default behavior after quiet_encoder_set_emit_blocking has
 * been called.
 */
//void quiet_encoder_set_emit_nonblocking(quiet_encoder *e);

/**
 * Clamp frame length to largest possible for sample length
 * @param e encoder object
 * @param sample_len size of sample block
 *
 * quiet_encoder_clamp_frame_len enables a mode in the encoder which prevents
 * data frames from overlapping multiple blocks of samples, e.g. multiple calls
 * to quiet_encoder_emit. This can be very convenient if your environment
 * cannot keep up in realtime due to e.g. GC pauses. The transmission of data
 * will succeed as long as the blocks of samples are played out smoothly (gaps
 * between blocks are ok, gaps within blocks are not ok).
 *
 * Calling this with the size of your sample block will clamp the frame length
 * of this encoder and toggle the `is_close_frame` flag which will ensure that
 * sample blocks will always end in silence. This will never result in a frame
 * length longer than the one provided in the creation of the encoder, but it
 * may result in a shorter frame length.
 *
 * @return the new frame length
 */
size_t quiet_encoder_clamp_frame_len(quiet_encoder *e, size_t sample_len);

/**
 * Retrieve encoder frame length
 * @param e encoder object
 *
 * @return encoder's maximum frame length, e.g. the largest length that can
 *  be passed to quiet_encoder_send
 */
size_t quiet_encoder_get_frame_len(const quiet_encoder *e);

/**
 * Emit samples
 * @param e encoder object
 * @param samplebuf user-provided array where samples will be written
 * @param samplebuf_len length of user-provided array
 *
 * quiet_encoder_emit fills a block of samples pointed to by samplebuf by
 * reading frames from its transmit queue and encoding them into sound by using
 * the configuration specified at creation. These samples can be written out
 * directly to a file or soundcard.
 *
 * If you are using a soundcard, you will have to carefully choose the sample
 * size block. Typically, the largest size is 16384 samples. Larger block sizes
 * will help hide uneven latencies in the encoding process and ensure smoother
 * transmission at the cost of longer latencies.
 *
 * quiet_encoder_emit may return fewer than the number of samples requested.
 * Unlike quiet_encoder_send, quiet_encoder_emit does not block, even when
 * blocking mode is enabled. This is because soundcard interfaces typically
 * require realtime sample generation.
 *
 * If quiet_encoder_emit returns 0, then the transmit queue is closed and
 * empty, and no future calls to quiet_encoder_emit will retrieve any more
 * samples.
 *
 * @return the number of samples written to samplebuf, which shall never
 *  exceed samplebuf_len. If the returned number of samples written is less
 *  than samplebuf_len, then the encoder has finished encoding the payload
 *  (its transmit queue is empty and all state has been flushed out). The user
 *  should 0-fill any remaining length if the block is to be transmitted.
 *
 * @return If quiet_encoder_emit returns a negative length, then it will set the
 *  quiet error. Most commonly, this will happen when the transmit queue
 *  is empty and there are no frames ready to send, but the queue is still
 *  open. If and only if the queue is *closed* and has been completely read,
 *  quiet_encoder_emit will return 0 to signal EOF.
 *
 * @error foo bar baz
 * @error qux quuux
 */
ssize_t quiet_encoder_emit(quiet_encoder *e, quiet_sample_t *samplebuf, size_t samplebuf_len);

/**
 * Close encoder
 * @param e encoder object
 *
 * quiet_encoder_close closes the encoder object. This has the effect of
 * rejecting any future calls to quiet_encoder_send. Any previously queued
 * frames will be written by quiet_encoder_emit. Once the send queue is empty,
 * quiet_encoder_emit will set last error to quiet_closed.
 *
 */
void quiet_encoder_close(quiet_encoder *e);

/**
 * Destroy encoder
 * @param e encoder object
 *
 * quiet_encoder_destroy releases all resources allocated by the quiet_encoder.
 * After calling this function, the user should not call any other encoder
 * functions on the quiet_encoder.
 */
void quiet_encoder_destroy(quiet_encoder *e);

/**
 * @struct quiet_decoder
 * Sound decoder
 */
struct quiet_decoder;
typedef struct quiet_decoder quiet_decoder;

/**
 * Create decoder
 * @param opt quiet_decoder_options containing decoder configuration
 * @param sample_rate Sample rate that decoder will consume at
 *
 * quiet_decoder_create creates and initializes a new libquiet decoder for a
 * given set of options and sample rate.
 *
 * It is recommended to use the default sample rate of your device in order
 * to avoid any possible implicit resampling, which can distort samples.
 *
 * @return pointer to new decoder object, or NULL if creation failed.
 */
quiet_decoder *quiet_decoder_create(const quiet_decoder_options *opt, float sample_rate);

/**
 * Try to receive a single frame
 * @param d decoder object
 * @param data user buffer which quiet will write received frame into
 * @param len length of user-supplied buffer
 *
 * quiet_decoder_recv reads one frame from the decoder's receive buffer. By
 * default, this is a nonblocking call and will fail quickly if no frames are
 * ready to be received. However, if quiet_decoder_set_blocking is called
 * prior to this call, then it will wait for as much as the timeout specified
 * there until it can read a frame.
 *
 * If the user's supplied buffer is smaller than the length of the received
 * frame, then only `len` bytes will be copied to `data`. The remaining bytes
 * will be discarded.
 *
 * This function will never return frames for which the checksum has failed.
 *
 * If libquiet was built and linked with pthread, then this function may be
 * called from any thread, and by multiple threads concurrently.
 *
 * quiet_decoder_recv will return 0 if the decoder has been closed and the
 * receive queue is empty.
 *
 * quiet_decoder_recv will return a negative value and set the last error
 * to quiet_timedout if blocking mode is enabled and no frame could be read
 * before the timeout.
 *
 * quiet_decoder_recv will return a negative value and set the last error
 * to quiet_would_block if nonblocking mode is enabled and no frame was
 * available.
 *
 * @return number of bytes written to buffer, 0 at EOF, or -1 if no frames
 * available
 */
ssize_t quiet_decoder_recv(quiet_decoder *d, uint8_t *data, size_t len);

/**
 * Set blocking mode of quiet_decoder_recv
 * @param d decoder object
 * @param sec time_t number of seconds to block for
 * @param nano long number of nanoseconds to block for
 *
 * quiet_decoder_set_blocking changes the behavior of quiet_decoder_recv so
 * that it will block until a frame can be read. It will block for
 * approximately (nano + 1000000000*sec) nanoseconds.
 *
 * If `sec` and `nano` are both 0, then quiet_decoder_recv will block
 * indefinitely until a frame is received.
 *
 * This function is only supported on systems with pthread. Calling
 * quiet_decoder_set_blocking on a host without pthread will assert
 * false.
 *
 */
void quiet_decoder_set_blocking(quiet_decoder *d, time_t sec, long nano);

/**
 * Set nonblocking mode of quiet_decoder_recv
 * @param d decoder object
 *
 * quiet_decoder_set_nonblocking changes the behavior of quiet_decoder_recv
 * so that it will not block if it cannot read a frame. This function
 * restores the default behavior after quiet_decoder_set_blocking has
 * been called.
 *
 */
void quiet_decoder_set_nonblocking(quiet_decoder *d);

/**
 * Feed received sound samples to decoder
 * @param d decoder object
 * @param samplebuf array of samples received from sound card
 * @param sample_len number of samples in samplebuf
 *
 * quiet_decoder_consume consumes sound samples and decodes them to frames.
 * These can be samples obtained directly from a sound file, a soundcard's
 * microphone, or any other source which can receive quiet_sample_t (float).
 *
 * If you are using a soundcard, it is recommended to use the largest block
 * size offered. Typically, this is 16384 samples. Larger block sizes will
 * help hide uneven latencies in the decoding process and ensure smoother
 * reception at the cost of longer latencies.
 *
 * @return number of samples consumed, or 0 if the decoder is closed
 */
ssize_t quiet_decoder_consume(quiet_decoder *d, const quiet_sample_t *samplebuf, size_t sample_len);

/**
 * Check if a frame is likely being received
 * @param d decoder object
 *
 * quiet_decoder_frame_in_progress determines if a frame is likely in the
 * process of being received. It inspects information in the decoding process
 * and will be relevant to the last call to quiet_decoder_consume. There
 * is no guarantee of accuracy from this function, and both false-negatives
 * and false-positives can occur.
 *
 * The output of this function can be useful to avoid collisions when two
 * pairs of encoders/decoders share the same channel, e.g. in half-duplex.
 *
 * This function must be called from the same thread which calls
 * quiet_decoder_consume.
 *
 * @return true if a frame is likely being received
 */
bool quiet_decoder_frame_in_progress(quiet_decoder *d);

/**
 * Flush existing state through decoder
 * @param d decoder object
 *
 * quiet_decoder_flush empties out all internal buffers and attempts to decode
 * them
 *
 * This function need only be called after the sound stream has stopped.
 * It is especially useful for reading from sound files where there are no
 * trailing samples to "push" the decoded data through the decoder's filters
 */
void quiet_decoder_flush(quiet_decoder *d);

/**
 * Close decoder
 * @param d decoder object
 *
 * quiet_decoder_close closes the decoder object. Future calls to
 * quiet_decoder_consume will still attempt the decoding process but
 * will not enqueue any decoded frames into the receive queue, e.g.
 * they become cpu-expensive no-ops. Any previously enqueued frames
 * can still be read out by quiet_decoder_recv, and once the receive queue
 * is empty, quiet_decoder_recv will set the last error to quiet_closed.
 *
 */
void quiet_decoder_close(quiet_decoder *d);

/**
 * Return number of failed frames
 * @param d decoder object
 *
 * quiet_decoder_checksum_fails returns the total number of frames decoded
 * but which failed checksum across the lifetime of the decoder.
 *
 * @return Total number of frames received with failed checksums
 */
unsigned int quiet_decoder_checksum_fails(const quiet_decoder *d);

/**
 * Fetch stats from last call to quiet_decoder_consume
 * @param d decoder object
 * @param num_frames number of frames at returned pointer
 *
 * quiet_decoder_consume_stats returns detailed info about the decoding
 * process from the last call to quiet_decoder_consume_stats. It will
 * save information on up to 8 frames. This includes frames which failed
 * checksum. If quiet_decoder_consume found more than 8 frames, then
 * information on only the first 8 frames will be saved.
 *
 * In order to use this functionality, quiet_decoder_enable_stats
 * must be called on the decoder object before calling
 * quiet_decoder_consume.
 *
 * This function must be called from the same thread that calls
 * quiet_decoder_consume.
 *
 * @return quiet_decoder_frame_stats which is an array of structs containing
 *  stats info, num_frames long
 */
const quiet_decoder_frame_stats *quiet_decoder_consume_stats(quiet_decoder *d, size_t *num_frames);

const quiet_decoder_frame_stats *quiet_decoder_recv_stats(quiet_decoder *d);

/**
 * Enable stats collection
 * @param d decoder object
 *
 * quiet_decoder_enable_stats allocates the required memory needed to save
 * symbol and other information on each frame decode. Italso adds a small
 * overhead needed to copy this information into an internal buffer.
 *
 * By default, stats collection is disabled. Therefore, if the user would like
 * to use quiet_decoder_consume_stats, then they must first call
 * quiet_decoder_enable_stats.
 */
void quiet_decoder_enable_stats(quiet_decoder *d);

/**
 * Disable stats collection
 * @param d decoder object
 *
 * quiet_decoder_disable_stats frees all memory associated with
 * stats collection.
 */
void quiet_decoder_disable_stats(quiet_decoder *d);

void quiet_decoder_set_stats_blocking(quiet_decoder *d, time_t sec, long nano);

void quiet_decoder_set_stats_nonblocking(quiet_decoder *d);

/*
 * $Id$
 * PortAudio Portable Real-Time Audio Library
 * PortAudio API Header File
 * Latest version available at: http://www.portaudio.com/
 *
 * Copyright (c) 1999-2002 Ross Bencina and Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortAudio license; however, 
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also 
 * requested that these non-binding requests be included along with the 
 * license above.
 */

/** @file
 @ingroup public_header
 @brief The portable PortAudio API.
*/


/** Retrieve the release number of the currently running PortAudio build.
 For example, for version "19.5.1" this will return 0x00130501.

 @see paMakeVersionNumber
*/
int Pa_GetVersion( void );

/** Retrieve a textual description of the current PortAudio build,
 e.g. "PortAudio V19.5.0-devel, revision 1952M".
 The format of the text may change in the future. Do not try to parse the
 returned string.

 @deprecated As of 19.5.0, use Pa_GetVersionInfo()->versionText instead.
*/
const char* Pa_GetVersionText( void );

/**
 Generate a packed integer version number in the same format used
 by Pa_GetVersion(). Use this to compare a specified version number with
 the currently running version. For example:

 @code
     if( Pa_GetVersion() < paMakeVersionNumber(19,5,1) ) {}
 @endcode

 @see Pa_GetVersion, Pa_GetVersionInfo
 @version Available as of 19.5.0.
*/
//#define paMakeVersionNumber(major, minor, subminor) \
//    (((major)&0xFF)<<16 | ((minor)&0xFF)<<8 | ((subminor)&0xFF))


/**
 A structure containing PortAudio API version information.
 @see Pa_GetVersionInfo, paMakeVersionNumber
 @version Available as of 19.5.0.
*/
typedef struct PaVersionInfo {
    int versionMajor;
    int versionMinor;
    int versionSubMinor;
    /**
     This is currently the Git revision hash but may change in the future.
     The versionControlRevision is updated by running a script before compiling the library.
     If the update does not occur, this value may refer to an earlier revision.
    */
    const char *versionControlRevision;
    /** Version as a string, for example "PortAudio V19.5.0-devel, revision 1952M" */
    const char *versionText;
} PaVersionInfo;
    
/** Retrieve version information for the currently running PortAudio build.
 @return A pointer to an immutable PaVersionInfo structure.

 @note This function can be called at any time. It does not require PortAudio
 to be initialized. The structure pointed to is statically allocated. Do not
 attempt to free it or modify it.

 @see PaVersionInfo, paMakeVersionNumber
 @version Available as of 19.5.0.
*/
//const PaVersionInfo* Pa_GetVersionInfo();


/** Error codes returned by PortAudio functions.
 Note that with the exception of paNoError, all PaErrorCodes are negative.
*/

typedef int PaError;
typedef enum PaErrorCode
{
    paNoError = 0,

    paNotInitialized = -10000,
    paUnanticipatedHostError,
    paInvalidChannelCount,
    paInvalidSampleRate,
    paInvalidDevice,
    paInvalidFlag,
    paSampleFormatNotSupported,
    paBadIODeviceCombination,
    paInsufficientMemory,
    paBufferTooBig,
    paBufferTooSmall,
    paNullCallback,
    paBadStreamPtr,
    paTimedOut,
    paInternalError,
    paDeviceUnavailable,
    paIncompatibleHostApiSpecificStreamInfo,
    paStreamIsStopped,
    paStreamIsNotStopped,
    paInputOverflowed,
    paOutputUnderflowed,
    paHostApiNotFound,
    paInvalidHostApi,
    paCanNotReadFromACallbackStream,
    paCanNotWriteToACallbackStream,
    paCanNotReadFromAnOutputOnlyStream,
    paCanNotWriteToAnInputOnlyStream,
    paIncompatibleStreamHostApi,
    paBadBufferPtr
} PaErrorCode;


/** Translate the supplied PortAudio error code into a human readable
 message.
*/
const char *Pa_GetErrorText( PaError errorCode );


/** Library initialization function - call this before using PortAudio.
 This function initializes internal data structures and prepares underlying
 host APIs for use.  With the exception of Pa_GetVersion(), Pa_GetVersionText(),
 and Pa_GetErrorText(), this function MUST be called before using any other
 PortAudio API functions.

 If Pa_Initialize() is called multiple times, each successful 
 call must be matched with a corresponding call to Pa_Terminate(). 
 Pairs of calls to Pa_Initialize()/Pa_Terminate() may overlap, and are not 
 required to be fully nested.

 Note that if Pa_Initialize() returns an error code, Pa_Terminate() should
 NOT be called.

 @return paNoError if successful, otherwise an error code indicating the cause
 of failure.

 @see Pa_Terminate
*/
PaError Pa_Initialize( void );


/** Library termination function - call this when finished using PortAudio.
 This function deallocates all resources allocated by PortAudio since it was
 initialized by a call to Pa_Initialize(). In cases where Pa_Initialise() has
 been called multiple times, each call must be matched with a corresponding call
 to Pa_Terminate(). The final matching call to Pa_Terminate() will automatically
 close any PortAudio streams that are still open.

 Pa_Terminate() MUST be called before exiting a program which uses PortAudio.
 Failure to do so may result in serious resource leaks, such as audio devices
 not being available until the next reboot.

 @return paNoError if successful, otherwise an error code indicating the cause
 of failure.
 
 @see Pa_Initialize
*/
PaError Pa_Terminate( void );



/** The type used to refer to audio devices. Values of this type usually
 range from 0 to (Pa_GetDeviceCount()-1), and may also take on the PaNoDevice
 and paUseHostApiSpecificDeviceSpecification values.

 @see Pa_GetDeviceCount, paNoDevice, paUseHostApiSpecificDeviceSpecification
*/
typedef int PaDeviceIndex;


/** A special PaDeviceIndex value indicating that no device is available,
 or should be used.

 @see PaDeviceIndex
*/
#define paNoDevice -1


/** A special PaDeviceIndex value indicating that the device(s) to be used
 are specified in the host api specific stream info structure.

 @see PaDeviceIndex
*/
#define paUseHostApiSpecificDeviceSpecification -2


/* Host API enumeration mechanism */

/** The type used to enumerate to host APIs at runtime. Values of this type
 range from 0 to (Pa_GetHostApiCount()-1).

 @see Pa_GetHostApiCount
*/
typedef int PaHostApiIndex;


/** Retrieve the number of available host APIs. Even if a host API is
 available it may have no devices available.

 @return A non-negative value indicating the number of available host APIs
 or, a PaErrorCode (which are always negative) if PortAudio is not initialized
 or an error is encountered.

 @see PaHostApiIndex
*/
PaHostApiIndex Pa_GetHostApiCount( void );


/** Retrieve the index of the default host API. The default host API will be
 the lowest common denominator host API on the current platform and is
 unlikely to provide the best performance.

 @return A non-negative value ranging from 0 to (Pa_GetHostApiCount()-1)
 indicating the default host API index or, a PaErrorCode (which are always
 negative) if PortAudio is not initialized or an error is encountered.
*/
PaHostApiIndex Pa_GetDefaultHostApi( void );


/** Unchanging unique identifiers for each supported host API. This type
 is used in the PaHostApiInfo structure. The values are guaranteed to be
 unique and to never change, thus allowing code to be written that
 conditionally uses host API specific extensions.

 New type ids will be allocated when support for a host API reaches
 "public alpha" status, prior to that developers should use the
 paInDevelopment type id.

 @see PaHostApiInfo
*/
typedef enum PaHostApiTypeId
{
    paInDevelopment=0, /* use while developing support for a new host API */
    paDirectSound=1,
    paMME=2,
    paASIO=3,
    paSoundManager=4,
    paCoreAudio=5,
    paOSS=7,
    paALSA=8,
    paAL=9,
    paBeOS=10,
    paWDMKS=11,
    paJACK=12,
    paWASAPI=13,
    paAudioScienceHPI=14
} PaHostApiTypeId;


/** A structure containing information about a particular host API. */

typedef struct PaHostApiInfo
{
    /** this is struct version 1 */
    int structVersion;
    /** The well known unique identifier of this host API @see PaHostApiTypeId */
    PaHostApiTypeId type;
    /** A textual description of the host API for display on user interfaces. */
    const char *name;

    /**  The number of devices belonging to this host API. This field may be
     used in conjunction with Pa_HostApiDeviceIndexToDeviceIndex() to enumerate
     all devices for this host API.
     @see Pa_HostApiDeviceIndexToDeviceIndex
    */
    int deviceCount;

    /** The default input device for this host API. The value will be a
     device index ranging from 0 to (Pa_GetDeviceCount()-1), or paNoDevice
     if no default input device is available.
    */
    PaDeviceIndex defaultInputDevice;

    /** The default output device for this host API. The value will be a
     device index ranging from 0 to (Pa_GetDeviceCount()-1), or paNoDevice
     if no default output device is available.
    */
    PaDeviceIndex defaultOutputDevice;
    
} PaHostApiInfo;


/** Retrieve a pointer to a structure containing information about a specific
 host Api.

 @param hostApi A valid host API index ranging from 0 to (Pa_GetHostApiCount()-1)

 @return A pointer to an immutable PaHostApiInfo structure describing
 a specific host API. If the hostApi parameter is out of range or an error
 is encountered, the function returns NULL.

 The returned structure is owned by the PortAudio implementation and must not
 be manipulated or freed. The pointer is only guaranteed to be valid between
 calls to Pa_Initialize() and Pa_Terminate().
*/
const PaHostApiInfo * Pa_GetHostApiInfo( PaHostApiIndex hostApi );


/** Convert a static host API unique identifier, into a runtime
 host API index.

 @param type A unique host API identifier belonging to the PaHostApiTypeId
 enumeration.

 @return A valid PaHostApiIndex ranging from 0 to (Pa_GetHostApiCount()-1) or,
 a PaErrorCode (which are always negative) if PortAudio is not initialized
 or an error is encountered.
 
 The paHostApiNotFound error code indicates that the host API specified by the
 type parameter is not available.

 @see PaHostApiTypeId
*/
PaHostApiIndex Pa_HostApiTypeIdToHostApiIndex( PaHostApiTypeId type );


/** Convert a host-API-specific device index to standard PortAudio device index.
 This function may be used in conjunction with the deviceCount field of
 PaHostApiInfo to enumerate all devices for the specified host API.

 @param hostApi A valid host API index ranging from 0 to (Pa_GetHostApiCount()-1)

 @param hostApiDeviceIndex A valid per-host device index in the range
 0 to (Pa_GetHostApiInfo(hostApi)->deviceCount-1)

 @return A non-negative PaDeviceIndex ranging from 0 to (Pa_GetDeviceCount()-1)
 or, a PaErrorCode (which are always negative) if PortAudio is not initialized
 or an error is encountered.

 A paInvalidHostApi error code indicates that the host API index specified by
 the hostApi parameter is out of range.

 A paInvalidDevice error code indicates that the hostApiDeviceIndex parameter
 is out of range.
 
 @see PaHostApiInfo
*/
PaDeviceIndex Pa_HostApiDeviceIndexToDeviceIndex( PaHostApiIndex hostApi,
        int hostApiDeviceIndex );



/** Structure used to return information about a host error condition.
*/
typedef struct PaHostErrorInfo{
    PaHostApiTypeId hostApiType;    /**< the host API which returned the error code */
    long errorCode;                 /**< the error code returned */
    const char *errorText;          /**< a textual description of the error if available, otherwise a zero-length string */
}PaHostErrorInfo;


/** Return information about the last host error encountered. The error
 information returned by Pa_GetLastHostErrorInfo() will never be modified
 asynchronously by errors occurring in other PortAudio owned threads
 (such as the thread that manages the stream callback.)

 This function is provided as a last resort, primarily to enhance debugging
 by providing clients with access to all available error information.

 @return A pointer to an immutable structure constraining information about
 the host error. The values in this structure will only be valid if a
 PortAudio function has previously returned the paUnanticipatedHostError
 error code.
*/
const PaHostErrorInfo* Pa_GetLastHostErrorInfo( void );



/* Device enumeration and capabilities */

/** Retrieve the number of available devices. The number of available devices
 may be zero.

 @return A non-negative value indicating the number of available devices or,
 a PaErrorCode (which are always negative) if PortAudio is not initialized
 or an error is encountered.
*/
PaDeviceIndex Pa_GetDeviceCount( void );


/** Retrieve the index of the default input device. The result can be
 used in the inputDevice parameter to Pa_OpenStream().

 @return The default input device index for the default host API, or paNoDevice
 if no default input device is available or an error was encountered.
*/
PaDeviceIndex Pa_GetDefaultInputDevice( void );


/** Retrieve the index of the default output device. The result can be
 used in the outputDevice parameter to Pa_OpenStream().

 @return The default output device index for the default host API, or paNoDevice
 if no default output device is available or an error was encountered.

 @note
 On the PC, the user can specify a default device by
 setting an environment variable. For example, to use device #1.
<pre>
 set PA_RECOMMENDED_OUTPUT_DEVICE=1
</pre>
 The user should first determine the available device ids by using
 the supplied application "pa_devs".
*/
PaDeviceIndex Pa_GetDefaultOutputDevice( void );


/** The type used to represent monotonic time in seconds. PaTime is 
 used for the fields of the PaStreamCallbackTimeInfo argument to the 
 PaStreamCallback and as the result of Pa_GetStreamTime().

 PaTime values have unspecified origin.
     
 @see PaStreamCallback, PaStreamCallbackTimeInfo, Pa_GetStreamTime
*/
typedef double PaTime;


/** A type used to specify one or more sample formats. Each value indicates
 a possible format for sound data passed to and from the stream callback,
 Pa_ReadStream and Pa_WriteStream.

 The standard formats paFloat32, paInt16, paInt32, paInt24, paInt8
 and aUInt8 are usually implemented by all implementations.

 The floating point representation (paFloat32) uses +1.0 and -1.0 as the
 maximum and minimum respectively.

 paUInt8 is an unsigned 8 bit format where 128 is considered "ground"

 The paNonInterleaved flag indicates that audio data is passed as an array 
 of pointers to separate buffers, one buffer for each channel. Usually,
 when this flag is not used, audio data is passed as a single buffer with
 all channels interleaved.

 @see Pa_OpenStream, Pa_OpenDefaultStream, PaDeviceInfo
 @see paFloat32, paInt16, paInt32, paInt24, paInt8
 @see paUInt8, paCustomFormat, paNonInterleaved
*/
typedef unsigned long PaSampleFormat;


#define paFloat32        0x00000001 /**< @see PaSampleFormat */
#define paInt32          0x00000002 /**< @see PaSampleFormat */
#define paInt24          0x00000004 /**< Packed 24 bit format. @see PaSampleFormat */
#define paInt16          0x00000008 /**< @see PaSampleFormat */
#define paInt8           0x00000010 /**< @see PaSampleFormat */
#define paUInt8          0x00000020 /**< @see PaSampleFormat */
#define paCustomFormat   0x00010000 /**< @see PaSampleFormat */

#define paNonInterleaved 0x80000000 /**< @see PaSampleFormat */

/** A structure providing information and capabilities of PortAudio devices.
 Devices may support input, output or both input and output.
*/
typedef struct PaDeviceInfo
{
    int structVersion;  /* this is struct version 2 */
    const char *name;
    PaHostApiIndex hostApi; /**< note this is a host API index, not a type id*/
    
    int maxInputChannels;
    int maxOutputChannels;

    /** Default latency values for interactive performance. */
    PaTime defaultLowInputLatency;
    PaTime defaultLowOutputLatency;
    /** Default latency values for robust non-interactive applications (eg. playing sound files). */
    PaTime defaultHighInputLatency;
    PaTime defaultHighOutputLatency;

    double defaultSampleRate;
} PaDeviceInfo;


/** Retrieve a pointer to a PaDeviceInfo structure containing information
 about the specified device.
 @return A pointer to an immutable PaDeviceInfo structure. If the device
 parameter is out of range the function returns NULL.

 @param device A valid device index in the range 0 to (Pa_GetDeviceCount()-1)

 @note PortAudio manages the memory referenced by the returned pointer,
 the client must not manipulate or free the memory. The pointer is only
 guaranteed to be valid between calls to Pa_Initialize() and Pa_Terminate().

 @see PaDeviceInfo, PaDeviceIndex
*/
const PaDeviceInfo* Pa_GetDeviceInfo( PaDeviceIndex device );


/** Parameters for one direction (input or output) of a stream.
*/
typedef struct PaStreamParameters
{
    /** A valid device index in the range 0 to (Pa_GetDeviceCount()-1)
     specifying the device to be used or the special constant
     paUseHostApiSpecificDeviceSpecification which indicates that the actual
     device(s) to use are specified in hostApiSpecificStreamInfo.
     This field must not be set to paNoDevice.
    */
    PaDeviceIndex device;
    
    /** The number of channels of sound to be delivered to the
     stream callback or accessed by Pa_ReadStream() or Pa_WriteStream().
     It can range from 1 to the value of maxInputChannels in the
     PaDeviceInfo record for the device specified by the device parameter.
    */
    int channelCount;

    /** The sample format of the buffer provided to the stream callback,
     a_ReadStream() or Pa_WriteStream(). It may be any of the formats described
     by the PaSampleFormat enumeration.
    */
    PaSampleFormat sampleFormat;

    /** The desired latency in seconds. Where practical, implementations should
     configure their latency based on these parameters, otherwise they may
     choose the closest viable latency instead. Unless the suggested latency
     is greater than the absolute upper limit for the device implementations
     should round the suggestedLatency up to the next practical value - ie to
     provide an equal or higher latency than suggestedLatency wherever possible.
     Actual latency values for an open stream may be retrieved using the
     inputLatency and outputLatency fields of the PaStreamInfo structure
     returned by Pa_GetStreamInfo().
     @see default*Latency in PaDeviceInfo, *Latency in PaStreamInfo
    */
    PaTime suggestedLatency;

    /** An optional pointer to a host api specific data structure
     containing additional information for device setup and/or stream processing.
     hostApiSpecificStreamInfo is never required for correct operation,
     if not used it should be set to NULL.
    */
    void *hostApiSpecificStreamInfo;

} PaStreamParameters;


/** Return code for Pa_IsFormatSupported indicating success. */
#define paFormatIsSupported 0

/** Determine whether it would be possible to open a stream with the specified
 parameters.

 @param inputParameters A structure that describes the input parameters used to
 open a stream. The suggestedLatency field is ignored. See PaStreamParameters
 for a description of these parameters. inputParameters must be NULL for
 output-only streams.

 @param outputParameters A structure that describes the output parameters used
 to open a stream. The suggestedLatency field is ignored. See PaStreamParameters
 for a description of these parameters. outputParameters must be NULL for
 input-only streams.

 @param sampleRate The required sampleRate. For full-duplex streams it is the
 sample rate for both input and output

 @return Returns 0 if the format is supported, and an error code indicating why
 the format is not supported otherwise. The constant paFormatIsSupported is
 provided to compare with the return value for success.

 @see paFormatIsSupported, PaStreamParameters
*/
PaError Pa_IsFormatSupported( const PaStreamParameters *inputParameters,
                              const PaStreamParameters *outputParameters,
                              double sampleRate );



/* Streaming types and functions */


/**
 A single PaStream can provide multiple channels of real-time
 streaming audio input and output to a client application. A stream
 provides access to audio hardware represented by one or more
 PaDevices. Depending on the underlying Host API, it may be possible 
 to open multiple streams using the same device, however this behavior 
 is implementation defined. Portable applications should assume that 
 a PaDevice may be simultaneously used by at most one PaStream.

 Pointers to PaStream objects are passed between PortAudio functions that
 operate on streams.

 @see Pa_OpenStream, Pa_OpenDefaultStream, Pa_OpenDefaultStream, Pa_CloseStream,
 Pa_StartStream, Pa_StopStream, Pa_AbortStream, Pa_IsStreamActive,
 Pa_GetStreamTime, Pa_GetStreamCpuLoad

*/
typedef void PaStream;


/** Can be passed as the framesPerBuffer parameter to Pa_OpenStream()
 or Pa_OpenDefaultStream() to indicate that the stream callback will
 accept buffers of any size.
*/
#define paFramesPerBufferUnspecified  0


/** Flags used to control the behavior of a stream. They are passed as
 parameters to Pa_OpenStream or Pa_OpenDefaultStream. Multiple flags may be
 ORed together.

 @see Pa_OpenStream, Pa_OpenDefaultStream
 @see paNoFlag, paClipOff, paDitherOff, paNeverDropInput,
  paPrimeOutputBuffersUsingStreamCallback, paPlatformSpecificFlags
*/
typedef unsigned long PaStreamFlags;

/** @see PaStreamFlags */
#define   paNoFlag          0

/** Disable default clipping of out of range samples.
 @see PaStreamFlags
*/
#define   paClipOff         0x00000001

/** Disable default dithering.
 @see PaStreamFlags
*/
#define   paDitherOff       0x00000002

/** Flag requests that where possible a full duplex stream will not discard
 overflowed input samples without calling the stream callback. This flag is
 only valid for full duplex callback streams and only when used in combination
 with the paFramesPerBufferUnspecified (0) framesPerBuffer parameter. Using
 this flag incorrectly results in a paInvalidFlag error being returned from
 Pa_OpenStream and Pa_OpenDefaultStream.

 @see PaStreamFlags, paFramesPerBufferUnspecified
*/
#define   paNeverDropInput  0x00000004

/** Call the stream callback to fill initial output buffers, rather than the
 default behavior of priming the buffers with zeros (silence). This flag has
 no effect for input-only and blocking read/write streams.
 
 @see PaStreamFlags
*/
#define   paPrimeOutputBuffersUsingStreamCallback 0x00000008

/** A mask specifying the platform specific bits.
 @see PaStreamFlags
*/
#define   paPlatformSpecificFlags 0xFFFF0000

/**
 Timing information for the buffers passed to the stream callback.

 Time values are expressed in seconds and are synchronised with the time base used by Pa_GetStreamTime() for the associated stream.
 
 @see PaStreamCallback, Pa_GetStreamTime
*/
typedef struct PaStreamCallbackTimeInfo{
    PaTime inputBufferAdcTime;  /**< The time when the first sample of the input buffer was captured at the ADC input */
    PaTime currentTime;         /**< The time when the stream callback was invoked */
    PaTime outputBufferDacTime; /**< The time when the first sample of the output buffer will output the DAC */
} PaStreamCallbackTimeInfo;


/**
 Flag bit constants for the statusFlags to PaStreamCallback.

 @see paInputUnderflow, paInputOverflow, paOutputUnderflow, paOutputOverflow,
 paPrimingOutput
*/
typedef unsigned long PaStreamCallbackFlags;

/** In a stream opened with paFramesPerBufferUnspecified, indicates that
 input data is all silence (zeros) because no real data is available. In a
 stream opened without paFramesPerBufferUnspecified, it indicates that one or
 more zero samples have been inserted into the input buffer to compensate
 for an input underflow.
 @see PaStreamCallbackFlags
*/
#define paInputUnderflow   0x00000001

/** In a stream opened with paFramesPerBufferUnspecified, indicates that data
 prior to the first sample of the input buffer was discarded due to an
 overflow, possibly because the stream callback is using too much CPU time.
 Otherwise indicates that data prior to one or more samples in the
 input buffer was discarded.
 @see PaStreamCallbackFlags
*/
#define paInputOverflow    0x00000002

/** Indicates that output data (or a gap) was inserted, possibly because the
 stream callback is using too much CPU time.
 @see PaStreamCallbackFlags
*/
#define paOutputUnderflow  0x00000004

/** Indicates that output data will be discarded because no room is available.
 @see PaStreamCallbackFlags
*/
#define paOutputOverflow   0x00000008

/** Some of all of the output data will be used to prime the stream, input
 data may be zero.
 @see PaStreamCallbackFlags
*/
#define paPrimingOutput    0x00000010

/**
 Allowable return values for the PaStreamCallback.
 @see PaStreamCallback
*/
typedef enum PaStreamCallbackResult
{
    paContinue=0,   /**< Signal that the stream should continue invoking the callback and processing audio. */
    paComplete=1,   /**< Signal that the stream should stop invoking the callback and finish once all output samples have played. */
    paAbort=2       /**< Signal that the stream should stop invoking the callback and finish as soon as possible. */
} PaStreamCallbackResult;


/**
 Functions of type PaStreamCallback are implemented by PortAudio clients.
 They consume, process or generate audio in response to requests from an
 active PortAudio stream.

 When a stream is running, PortAudio calls the stream callback periodically.
 The callback function is responsible for processing buffers of audio samples 
 passed via the input and output parameters.

 The PortAudio stream callback runs at very high or real-time priority.
 It is required to consistently meet its time deadlines. Do not allocate 
 memory, access the file system, call library functions or call other functions 
 from the stream callback that may block or take an unpredictable amount of
 time to complete.

 In order for a stream to maintain glitch-free operation the callback
 must consume and return audio data faster than it is recorded and/or
 played. PortAudio anticipates that each callback invocation may execute for 
 a duration approaching the duration of frameCount audio frames at the stream 
 sample rate. It is reasonable to expect to be able to utilise 70% or more of
 the available CPU time in the PortAudio callback. However, due to buffer size 
 adaption and other factors, not all host APIs are able to guarantee audio 
 stability under heavy CPU load with arbitrary fixed callback buffer sizes. 
 When high callback CPU utilisation is required the most robust behavior 
 can be achieved by using paFramesPerBufferUnspecified as the 
 Pa_OpenStream() framesPerBuffer parameter.
     
 @param input and @param output are either arrays of interleaved samples or;
 if non-interleaved samples were requested using the paNonInterleaved sample 
 format flag, an array of buffer pointers, one non-interleaved buffer for 
 each channel.

 The format, packing and number of channels used by the buffers are
 determined by parameters to Pa_OpenStream().
     
 @param frameCount The number of sample frames to be processed by
 the stream callback.

 @param timeInfo Timestamps indicating the ADC capture time of the first sample
 in the input buffer, the DAC output time of the first sample in the output buffer
 and the time the callback was invoked. 
 See PaStreamCallbackTimeInfo and Pa_GetStreamTime()

 @param statusFlags Flags indicating whether input and/or output buffers
 have been inserted or will be dropped to overcome underflow or overflow
 conditions.

 @param userData The value of a user supplied pointer passed to
 Pa_OpenStream() intended for storing synthesis data etc.

 @return
 The stream callback should return one of the values in the
 ::PaStreamCallbackResult enumeration. To ensure that the callback continues
 to be called, it should return paContinue (0). Either paComplete or paAbort
 can be returned to finish stream processing, after either of these values is
 returned the callback will not be called again. If paAbort is returned the
 stream will finish as soon as possible. If paComplete is returned, the stream
 will continue until all buffers generated by the callback have been played.
 This may be useful in applications such as soundfile players where a specific
 duration of output is required. However, it is not necessary to utilize this
 mechanism as Pa_StopStream(), Pa_AbortStream() or Pa_CloseStream() can also
 be used to stop the stream. The callback must always fill the entire output
 buffer irrespective of its return value.

 @see Pa_OpenStream, Pa_OpenDefaultStream

 @note With the exception of Pa_GetStreamCpuLoad() it is not permissible to call
 PortAudio API functions from within the stream callback.
*/
typedef int PaStreamCallback(
    const void *input, void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData );


/** Opens a stream for either input, output or both.
     
 @param stream The address of a PaStream pointer which will receive
 a pointer to the newly opened stream.
     
 @param inputParameters A structure that describes the input parameters used by
 the opened stream. See PaStreamParameters for a description of these parameters.
 inputParameters must be NULL for output-only streams.

 @param outputParameters A structure that describes the output parameters used by
 the opened stream. See PaStreamParameters for a description of these parameters.
 outputParameters must be NULL for input-only streams.
 
 @param sampleRate The desired sampleRate. For full-duplex streams it is the
 sample rate for both input and output
     
 @param framesPerBuffer The number of frames passed to the stream callback
 function, or the preferred block granularity for a blocking read/write stream.
 The special value paFramesPerBufferUnspecified (0) may be used to request that
 the stream callback will receive an optimal (and possibly varying) number of
 frames based on host requirements and the requested latency settings.
 Note: With some host APIs, the use of non-zero framesPerBuffer for a callback
 stream may introduce an additional layer of buffering which could introduce
 additional latency. PortAudio guarantees that the additional latency
 will be kept to the theoretical minimum however, it is strongly recommended
 that a non-zero framesPerBuffer value only be used when your algorithm
 requires a fixed number of frames per stream callback.
 
 @param streamFlags Flags which modify the behavior of the streaming process.
 This parameter may contain a combination of flags ORed together. Some flags may
 only be relevant to certain buffer formats.
     
 @param streamCallback A pointer to a client supplied function that is responsible
 for processing and filling input and output buffers. If this parameter is NULL
 the stream will be opened in 'blocking read/write' mode. In blocking mode,
 the client can receive sample data using Pa_ReadStream and write sample data
 using Pa_WriteStream, the number of samples that may be read or written
 without blocking is returned by Pa_GetStreamReadAvailable and
 Pa_GetStreamWriteAvailable respectively.

 @param userData A client supplied pointer which is passed to the stream callback
 function. It could for example, contain a pointer to instance data necessary
 for processing the audio buffers. This parameter is ignored if streamCallback
 is NULL.
     
 @return
 Upon success Pa_OpenStream() returns paNoError and places a pointer to a
 valid PaStream in the stream argument. The stream is inactive (stopped).
 If a call to Pa_OpenStream() fails, a non-zero error code is returned (see
 PaError for possible error codes) and the value of stream is invalid.

 @see PaStreamParameters, PaStreamCallback, Pa_ReadStream, Pa_WriteStream,
 Pa_GetStreamReadAvailable, Pa_GetStreamWriteAvailable
*/
PaError Pa_OpenStream( PaStream** stream,
                       const PaStreamParameters *inputParameters,
                       const PaStreamParameters *outputParameters,
                       double sampleRate,
                       unsigned long framesPerBuffer,
                       PaStreamFlags streamFlags,
                       PaStreamCallback *streamCallback,
                       void *userData );


/** A simplified version of Pa_OpenStream() that opens the default input
 and/or output devices.

 @param stream The address of a PaStream pointer which will receive
 a pointer to the newly opened stream.
 
 @param numInputChannels  The number of channels of sound that will be supplied
 to the stream callback or returned by Pa_ReadStream. It can range from 1 to
 the value of maxInputChannels in the PaDeviceInfo record for the default input
 device. If 0 the stream is opened as an output-only stream.

 @param numOutputChannels The number of channels of sound to be delivered to the
 stream callback or passed to Pa_WriteStream. It can range from 1 to the value
 of maxOutputChannels in the PaDeviceInfo record for the default output device.
 If 0 the stream is opened as an output-only stream.

 @param sampleFormat The sample format of both the input and output buffers
 provided to the callback or passed to and from Pa_ReadStream and Pa_WriteStream.
 sampleFormat may be any of the formats described by the PaSampleFormat
 enumeration.
 
 @param sampleRate Same as Pa_OpenStream parameter of the same name.
 @param framesPerBuffer Same as Pa_OpenStream parameter of the same name.
 @param streamCallback Same as Pa_OpenStream parameter of the same name.
 @param userData Same as Pa_OpenStream parameter of the same name.

 @return As for Pa_OpenStream

 @see Pa_OpenStream, PaStreamCallback
*/
PaError Pa_OpenDefaultStream( PaStream** stream,
                              int numInputChannels,
                              int numOutputChannels,
                              PaSampleFormat sampleFormat,
                              double sampleRate,
                              unsigned long framesPerBuffer,
                              PaStreamCallback *streamCallback,
                              void *userData );


/** Closes an audio stream. If the audio stream is active it
 discards any pending buffers as if Pa_AbortStream() had been called.
*/
PaError Pa_CloseStream( PaStream *stream );


/** Functions of type PaStreamFinishedCallback are implemented by PortAudio 
 clients. They can be registered with a stream using the Pa_SetStreamFinishedCallback
 function. Once registered they are called when the stream becomes inactive
 (ie once a call to Pa_StopStream() will not block).
 A stream will become inactive after the stream callback returns non-zero,
 or when Pa_StopStream or Pa_AbortStream is called. For a stream providing audio
 output, if the stream callback returns paComplete, or Pa_StopStream() is called,
 the stream finished callback will not be called until all generated sample data
 has been played.
 
 @param userData The userData parameter supplied to Pa_OpenStream()

 @see Pa_SetStreamFinishedCallback
*/
typedef void PaStreamFinishedCallback( void *userData );


/** Register a stream finished callback function which will be called when the 
 stream becomes inactive. See the description of PaStreamFinishedCallback for 
 further details about when the callback will be called.

 @param stream a pointer to a PaStream that is in the stopped state - if the
 stream is not stopped, the stream's finished callback will remain unchanged 
 and an error code will be returned.

 @param streamFinishedCallback a pointer to a function with the same signature
 as PaStreamFinishedCallback, that will be called when the stream becomes
 inactive. Passing NULL for this parameter will un-register a previously
 registered stream finished callback function.

 @return on success returns paNoError, otherwise an error code indicating the cause
 of the error.

 @see PaStreamFinishedCallback
*/
PaError Pa_SetStreamFinishedCallback( PaStream *stream, PaStreamFinishedCallback* streamFinishedCallback ); 


/** Commences audio processing.
*/
PaError Pa_StartStream( PaStream *stream );


/** Terminates audio processing. It waits until all pending
 audio buffers have been played before it returns.
*/
PaError Pa_StopStream( PaStream *stream );


/** Terminates audio processing immediately without waiting for pending
 buffers to complete.
*/
PaError Pa_AbortStream( PaStream *stream );


/** Determine whether the stream is stopped.
 A stream is considered to be stopped prior to a successful call to
 Pa_StartStream and after a successful call to Pa_StopStream or Pa_AbortStream.
 If a stream callback returns a value other than paContinue the stream is NOT
 considered to be stopped.

 @return Returns one (1) when the stream is stopped, zero (0) when
 the stream is running or, a PaErrorCode (which are always negative) if
 PortAudio is not initialized or an error is encountered.

 @see Pa_StopStream, Pa_AbortStream, Pa_IsStreamActive
*/
PaError Pa_IsStreamStopped( PaStream *stream );


/** Determine whether the stream is active.
 A stream is active after a successful call to Pa_StartStream(), until it
 becomes inactive either as a result of a call to Pa_StopStream() or
 Pa_AbortStream(), or as a result of a return value other than paContinue from
 the stream callback. In the latter case, the stream is considered inactive
 after the last buffer has finished playing.

 @return Returns one (1) when the stream is active (ie playing or recording
 audio), zero (0) when not playing or, a PaErrorCode (which are always negative)
 if PortAudio is not initialized or an error is encountered.

 @see Pa_StopStream, Pa_AbortStream, Pa_IsStreamStopped
*/
PaError Pa_IsStreamActive( PaStream *stream );



/** A structure containing unchanging information about an open stream.
 @see Pa_GetStreamInfo
*/

typedef struct PaStreamInfo
{
    /** this is struct version 1 */
    int structVersion;

    /** The input latency of the stream in seconds. This value provides the most
     accurate estimate of input latency available to the implementation. It may
     differ significantly from the suggestedLatency value passed to Pa_OpenStream().
     The value of this field will be zero (0.) for output-only streams.
     @see PaTime
    */
    PaTime inputLatency;

    /** The output latency of the stream in seconds. This value provides the most
     accurate estimate of output latency available to the implementation. It may
     differ significantly from the suggestedLatency value passed to Pa_OpenStream().
     The value of this field will be zero (0.) for input-only streams.
     @see PaTime
    */
    PaTime outputLatency;

    /** The sample rate of the stream in Hertz (samples per second). In cases
     where the hardware sample rate is inaccurate and PortAudio is aware of it,
     the value of this field may be different from the sampleRate parameter
     passed to Pa_OpenStream(). If information about the actual hardware sample
     rate is not available, this field will have the same value as the sampleRate
     parameter passed to Pa_OpenStream().
    */
    double sampleRate;
    
} PaStreamInfo;


/** Retrieve a pointer to a PaStreamInfo structure containing information
 about the specified stream.
 @return A pointer to an immutable PaStreamInfo structure. If the stream
 parameter is invalid, or an error is encountered, the function returns NULL.

 @param stream A pointer to an open stream previously created with Pa_OpenStream.

 @note PortAudio manages the memory referenced by the returned pointer,
 the client must not manipulate or free the memory. The pointer is only
 guaranteed to be valid until the specified stream is closed.

 @see PaStreamInfo
*/
const PaStreamInfo* Pa_GetStreamInfo( PaStream *stream );


/** Returns the current time in seconds for a stream according to the same clock used
 to generate callback PaStreamCallbackTimeInfo timestamps. The time values are
 monotonically increasing and have unspecified origin. 
 
 Pa_GetStreamTime returns valid time values for the entire life of the stream,
 from when the stream is opened until it is closed. Starting and stopping the stream
 does not affect the passage of time returned by Pa_GetStreamTime.

 This time may be used for synchronizing other events to the audio stream, for 
 example synchronizing audio to MIDI.
                                        
 @return The stream's current time in seconds, or 0 if an error occurred.

 @see PaTime, PaStreamCallback, PaStreamCallbackTimeInfo
*/
PaTime Pa_GetStreamTime( PaStream *stream );


/** Retrieve CPU usage information for the specified stream.
 The "CPU Load" is a fraction of total CPU time consumed by a callback stream's
 audio processing routines including, but not limited to the client supplied
 stream callback. This function does not work with blocking read/write streams.

 This function may be called from the stream callback function or the
 application.
     
 @return
 A floating point value, typically between 0.0 and 1.0, where 1.0 indicates
 that the stream callback is consuming the maximum number of CPU cycles possible
 to maintain real-time operation. A value of 0.5 would imply that PortAudio and
 the stream callback was consuming roughly 50% of the available CPU time. The
 return value may exceed 1.0. A value of 0.0 will always be returned for a
 blocking read/write stream, or if an error occurs.
*/
double Pa_GetStreamCpuLoad( PaStream* stream );


/** Read samples from an input stream. The function doesn't return until
 the entire buffer has been filled - this may involve waiting for the operating
 system to supply the data.

 @param stream A pointer to an open stream previously created with Pa_OpenStream.
 
 @param buffer A pointer to a buffer of sample frames. The buffer contains
 samples in the format specified by the inputParameters->sampleFormat field
 used to open the stream, and the number of channels specified by
 inputParameters->numChannels. If non-interleaved samples were requested using
 the paNonInterleaved sample format flag, buffer is a pointer to the first element 
 of an array of buffer pointers, one non-interleaved buffer for each channel.

 @param frames The number of frames to be read into buffer. This parameter
 is not constrained to a specific range, however high performance applications
 will want to match this parameter to the framesPerBuffer parameter used
 when opening the stream.

 @return On success PaNoError will be returned, or PaInputOverflowed if input
 data was discarded by PortAudio after the previous call and before this call.
*/
PaError Pa_ReadStream( PaStream* stream,
                       void *buffer,
                       unsigned long frames );


/** Write samples to an output stream. This function doesn't return until the
 entire buffer has been written - this may involve waiting for the operating
 system to consume the data.

 @param stream A pointer to an open stream previously created with Pa_OpenStream.

 @param buffer A pointer to a buffer of sample frames. The buffer contains
 samples in the format specified by the outputParameters->sampleFormat field
 used to open the stream, and the number of channels specified by
 outputParameters->numChannels. If non-interleaved samples were requested using
 the paNonInterleaved sample format flag, buffer is a pointer to the first element 
 of an array of buffer pointers, one non-interleaved buffer for each channel.

 @param frames The number of frames to be written from buffer. This parameter
 is not constrained to a specific range, however high performance applications
 will want to match this parameter to the framesPerBuffer parameter used
 when opening the stream.

 @return On success PaNoError will be returned, or paOutputUnderflowed if
 additional output data was inserted after the previous call and before this
 call.
*/
PaError Pa_WriteStream( PaStream* stream,
                        const void *buffer,
                        unsigned long frames );


/** Retrieve the number of frames that can be read from the stream without
 waiting.

 @return Returns a non-negative value representing the maximum number of frames
 that can be read from the stream without blocking or busy waiting or, a
 PaErrorCode (which are always negative) if PortAudio is not initialized or an
 error is encountered.
*/
signed long Pa_GetStreamReadAvailable( PaStream* stream );


/** Retrieve the number of frames that can be written to the stream without
 waiting.

 @return Returns a non-negative value representing the maximum number of frames
 that can be written to the stream without blocking or busy waiting or, a
 PaErrorCode (which are always negative) if PortAudio is not initialized or an
 error is encountered.
*/
signed long Pa_GetStreamWriteAvailable( PaStream* stream );


/** Retrieve the host type handling an open stream.

 @return Returns a non-negative value representing the host API type
 handling an open stream or, a PaErrorCode (which are always negative)
 if PortAudio is not initialized or an error is encountered.
*/
PaHostApiTypeId Pa_GetStreamHostApiType( PaStream* stream );


/* Miscellaneous utilities */


/** Retrieve the size of a given sample format in bytes.

 @return The size in bytes of a single sample in the specified format,
 or paSampleFormatNotSupported if the format is not supported.
*/
PaError Pa_GetSampleSize( PaSampleFormat format );


/** Put the caller to sleep for at least 'msec' milliseconds. This function is
 provided only as a convenience for authors of portable code (such as the tests
 and examples in the PortAudio distribution.)

 The function may sleep longer than requested so don't rely on this for accurate
 musical timing.
*/
void Pa_Sleep( long msec );




/**
 * Destroy decoder
 * @param d decoder object
 *
 * quiet_decoder_destroy releases all resources allocated by the quiet_decoder.
 * After calling this function, the user should not call any other decoder
 * functions on the decoder.
 */
void quiet_decoder_destroy(quiet_decoder *d);

// Sound encoder
struct quiet_portaudio_encoder;
typedef struct quiet_portaudio_encoder quiet_portaudio_encoder;

quiet_portaudio_encoder *quiet_portaudio_encoder_create(const quiet_encoder_options *opt, PaDeviceIndex device, PaTime latency, double sample_rate, size_t sample_buffer_size);

void quiet_portaudio_encoder_set_blocking(quiet_portaudio_encoder *e, time_t sec, long nano);

void quiet_portaudio_encoder_set_nonblocking(quiet_portaudio_encoder *e);

size_t quiet_portaudio_encoder_get_frame_len(const quiet_portaudio_encoder *e);

size_t quiet_portaudio_encoder_clamp_frame_len(quiet_portaudio_encoder *e, size_t sample_len);

ssize_t quiet_portaudio_encoder_send(quiet_portaudio_encoder *enc, const uint8_t *buf, size_t len);

ssize_t quiet_portaudio_encoder_emit(quiet_portaudio_encoder *enc);

void quiet_portaudio_encoder_emit_empty(quiet_portaudio_encoder *enc);

void quiet_portaudio_encoder_close(quiet_portaudio_encoder *enc);

void quiet_portaudio_encoder_destroy(quiet_portaudio_encoder *enc);

// Sound decoder
struct quiet_portaudio_decoder;
typedef struct quiet_portaudio_decoder quiet_portaudio_decoder;

quiet_portaudio_decoder *quiet_portaudio_decoder_create(const quiet_decoder_options *opt, PaDeviceIndex device, PaTime latency, double sample_rate);

ssize_t quiet_portaudio_decoder_recv(quiet_portaudio_decoder *d, uint8_t *data, size_t len);

void quiet_portaudio_decoder_set_blocking(quiet_portaudio_decoder *d, time_t sec, long nano);

void quiet_portaudio_decoder_set_nonblocking(quiet_portaudio_decoder *d);

bool quiet_portaudio_decoder_frame_in_progress(quiet_portaudio_decoder *d);

unsigned int quiet_portaudio_decoder_checksum_fails(const quiet_portaudio_decoder *d);

const quiet_decoder_frame_stats *quiet_portaudio_decoder_consume_stats(quiet_portaudio_decoder *d, size_t *num_frames);

void quiet_portaudio_decoder_enable_stats(quiet_portaudio_decoder *d);

void quiet_portaudio_decoder_disable_stats(quiet_portaudio_decoder *d);

void quiet_portaudio_decoder_close(quiet_portaudio_decoder *d);

void quiet_portaudio_decoder_destroy(quiet_portaudio_decoder *d);

