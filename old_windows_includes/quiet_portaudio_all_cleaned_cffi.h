
typedef float quiet_sample_t;
typedef ptrdiff_t ssize_t;
typedef long long time_t;
typedef int PaDeviceIndex;
typedef double PaTime;

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

quiet_error quiet_get_last_error();

typedef struct { float alpha; } quiet_dc_filter_options;

typedef struct {
    size_t delay;
    float bandwidth;
    float attenuation;
    size_t filter_bank_size;
} quiet_resampler_options;

typedef struct {
    unsigned int shape;
    unsigned int samples_per_symbol;
    unsigned int symbol_delay;
    float excess_bw;
    float center_rads;
    float gain;
    quiet_dc_filter_options dc_filter_opt;
} quiet_modulator_options;

typedef struct {
    unsigned int shape;
    unsigned int samples_per_symbol;
    unsigned int symbol_delay;
    float excess_bw;
    float center_rads;
} quiet_demodulator_options;

typedef enum quiet_checksum_schemes {
    quiet_checksum_none = 1,
    quiet_checksum_8bit,
    quiet_checksum_crc8,
    quiet_checksum_crc16,
    quiet_checksum_crc24,
    quiet_checksum_crc32,
} quiet_checksum_scheme_t;

typedef enum quiet_error_correction_schemes {
    quiet_error_correction_none = 1,
    quiet_error_correction_repeat_3,
    quiet_error_correction_repeat_5,
    quiet_error_correction_hamming_7_4,
    quiet_error_correction_hamming_7_4_parity,
    quiet_error_correction_hamming_12_8,
    quiet_error_correction_golay_24_12,
    quiet_error_correction_secded_22_16,
    quiet_error_correction_secded_39_32,
    quiet_error_correction_secded_72_64,
    quiet_error_correction_conv_12_7,
    quiet_error_correction_conv_12_9,
    quiet_error_correction_conv_13_9,
    quiet_error_correction_conv_16_15,
    quiet_error_correction_conv_perf_23_7,
    quiet_error_correction_conv_perf_34_7,
    quiet_error_correction_conv_perf_45_7,
    quiet_error_correction_conv_perf_56_7,
    quiet_error_correction_conv_perf_67_7,
    quiet_error_correction_conv_perf_78_7,
    quiet_error_correction_conv_perf_23_9,
    quiet_error_correction_conv_perf_34_9,
    quiet_error_correction_conv_perf_45_9,
    quiet_error_correction_conv_perf_56_9,
    quiet_error_correction_conv_perf_67_9,
    quiet_error_correction_conv_perf_78_9,
    quiet_error_correction_reed_solomon_223_255
} quiet_error_correction_scheme_t;

typedef enum quiet_modulation_schemes {
    quiet_modulation_psk2 = 1,
    quiet_modulation_psk4,
    quiet_modulation_psk8,
    quiet_modulation_psk16,
    quiet_modulation_psk32,
    quiet_modulation_psk64,
    quiet_modulation_psk128,
    quiet_modulation_psk256,
    quiet_modulation_dpsk2,
    quiet_modulation_dpsk4,
    quiet_modulation_dpsk8,
    quiet_modulation_dpsk16,
    quiet_modulation_dpsk32,
    quiet_modulation_dpsk64,
    quiet_modulation_dpsk128,
    quiet_modulation_dpsk256,
    quiet_modulation_ask2,
    quiet_modulation_ask4,
    quiet_modulation_ask8,
    quiet_modulation_ask16,
    quiet_modulation_ask32,
    quiet_modulation_ask64,
    quiet_modulation_ask128,
    quiet_modulation_ask256,
    quiet_modulation_qask4,
    quiet_modulation_qask8,
    quiet_modulation_qask16,
    quiet_modulation_qask32,
    quiet_modulation_qask64,
    quiet_modulation_qask128,
    quiet_modulation_qask256,
    quiet_modulation_qask512,
    quiet_modulation_qask1024,
    quiet_modulation_qask2048,
    quiet_modulation_qask4096,
    quiet_modulation_qask8192,
    quiet_modulation_qask16384,
    quiet_modulation_qask32768,
    quiet_modulation_qask65536,
    quiet_modulation_apsk4,
    quiet_modulation_apsk8,
    quiet_modulation_apsk16,
    quiet_modulation_apsk32,
    quiet_modulation_apsk64,
    quiet_modulation_apsk128,
    quiet_modulation_apsk256,
    quiet_modulation_bpsk,
    quiet_modulation_qpsk,
    quiet_modulation_ook,
    quiet_modulation_sqask32,
    quiet_modulation_sqask128,
    quiet_modulation_v29,
    quiet_modulation_opt_qask16,
    quiet_modulation_opt_qask32,
    quiet_modulation_opt_qask64,
    quiet_modulation_opt_qask128,
    quiet_modulation_opt_qask256,
    quiet_modulation_vtech,
} quiet_modulation_scheme_t;

typedef struct {
    unsigned int num_subcarriers;
    unsigned int cyclic_prefix_len;
    unsigned int taper_len;
    size_t left_band;
    size_t right_band;
} quiet_ofdm_options;

typedef enum encodings {
    ofdm_encoding,
    modem_encoding,
    gmsk_encoding,
} quiet_encoding_t;

typedef struct {
    quiet_ofdm_options ofdmopt;
    quiet_modulator_options modopt;
    quiet_resampler_options resampler;
    quiet_encoding_t encoding;
    quiet_checksum_scheme_t checksum_scheme;
    quiet_error_correction_scheme_t inner_fec_scheme;
    quiet_error_correction_scheme_t outer_fec_scheme;
    quiet_modulation_scheme_t mod_scheme;
    bool header_override_defaults;
    quiet_checksum_scheme_t header_checksum_scheme;
    quiet_error_correction_scheme_t header_inner_fec_scheme;
    quiet_error_correction_scheme_t header_outer_fec_scheme;
    quiet_modulation_scheme_t header_mod_scheme;
    size_t frame_len;
} quiet_encoder_options;

typedef struct {
    quiet_ofdm_options ofdmopt;
    quiet_demodulator_options demodopt;
    quiet_resampler_options resampler;
    quiet_encoding_t encoding;
    bool header_override_defaults;
    quiet_checksum_scheme_t header_checksum_scheme;
    quiet_error_correction_scheme_t header_inner_fec_scheme;
    quiet_error_correction_scheme_t header_outer_fec_scheme;
    quiet_modulation_scheme_t header_mod_scheme;
    bool is_debug;
} quiet_decoder_options;

typedef struct {
    float real;
    float imaginary;
} quiet_complex;

typedef struct {
    const quiet_complex *symbols;
    size_t num_symbols;
    float error_vector_magnitude;
    float received_signal_strength_indicator;
    bool checksum_passed;
} quiet_decoder_frame_stats;
quiet_decoder_options *quiet_decoder_profile_file(FILE *f, const char *profilename);
quiet_decoder_options *quiet_decoder_profile_filename(const char *fname, const char *profilename);
quiet_decoder_options *quiet_decoder_profile_str(const char *input, const char *profilename);
quiet_encoder_options *quiet_encoder_profile_file(FILE *f, const char *profilename);
quiet_encoder_options *quiet_encoder_profile_filename(const char *fname, const char *profilename);
quiet_encoder_options *quiet_encoder_profile_str(const char *input, const char *profilename);
char **quiet_profile_keys_file(FILE *f, size_t *numkeys);
char **quiet_profile_keys_filename(const char *fname, size_t *numkeys);
char **quiet_profile_keys_str(const char *input, size_t *numkeys);
struct quiet_encoder;
typedef struct quiet_encoder quiet_encoder;
quiet_encoder *quiet_encoder_create(const quiet_encoder_options *opt, float sample_rate);
ssize_t quiet_encoder_send(quiet_encoder *e, const void *buf, size_t len);
void quiet_encoder_set_blocking(quiet_encoder *e, time_t sec, long nano);
void quiet_encoder_set_nonblocking(quiet_encoder *e);
size_t quiet_encoder_clamp_frame_len(quiet_encoder *e, size_t sample_len);
size_t quiet_encoder_get_frame_len(const quiet_encoder *e);
ssize_t quiet_encoder_emit(quiet_encoder *e, quiet_sample_t *samplebuf, size_t samplebuf_len);
void quiet_encoder_close(quiet_encoder *e);
void quiet_encoder_destroy(quiet_encoder *e);
struct quiet_decoder;
typedef struct quiet_decoder quiet_decoder;
quiet_decoder *quiet_decoder_create(const quiet_decoder_options *opt, float sample_rate);
ssize_t quiet_decoder_recv(quiet_decoder *d, uint8_t *data, size_t len);
void quiet_decoder_set_blocking(quiet_decoder *d, time_t sec, long nano);
void quiet_decoder_set_nonblocking(quiet_decoder *d);
void quiet_decoder_consume(quiet_decoder *d, const quiet_sample_t *samplebuf, size_t sample_len);
bool quiet_decoder_frame_in_progress(quiet_decoder *d);
void quiet_decoder_flush(quiet_decoder *d);
void quiet_decoder_close(quiet_decoder *d);
unsigned int quiet_decoder_checksum_fails(const quiet_decoder *d);
const quiet_decoder_frame_stats *quiet_decoder_consume_stats(quiet_decoder *d, size_t *num_frames);
const quiet_decoder_frame_stats *quiet_decoder_recv_stats(quiet_decoder *d);
void quiet_decoder_enable_stats(quiet_decoder *d);
void quiet_decoder_disable_stats(quiet_decoder *d);
void quiet_decoder_set_stats_blocking(quiet_decoder *d, time_t sec, long nano);
void quiet_decoder_set_stats_nonblocking(quiet_decoder *d);
void quiet_decoder_destroy(quiet_decoder *d);

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
struct quiet_portaudio_decoder;
typedef struct quiet_portaudio_decoder quiet_portaudio_decoder;
quiet_portaudio_decoder *quiet_portaudio_decoder_create(const quiet_decoder_options *opt, PaDeviceIndex device, PaTime latency, double sample_rate, size_t sample_buffer_size);
ssize_t quiet_portaudio_decoder_recv(quiet_portaudio_decoder *d, uint8_t *data, size_t len);
void quiet_portaudio_decoder_set_blocking(quiet_portaudio_decoder *d, time_t sec, long nano);
void quiet_portaudio_decoder_set_nonblocking(quiet_portaudio_decoder *d);
void quiet_portaudio_decoder_consume(quiet_portaudio_decoder *d);
bool quiet_portaudio_decoder_frame_in_progress(quiet_portaudio_decoder *d);
unsigned int quiet_portaudio_decoder_checksum_fails(const quiet_portaudio_decoder *d);
const quiet_decoder_frame_stats *quiet_portaudio_decoder_consume_stats(quiet_portaudio_decoder *d, size_t *num_frames);
void quiet_portaudio_decoder_enable_stats(quiet_portaudio_decoder *d);
void quiet_portaudio_decoder_disable_stats(quiet_portaudio_decoder *d);
void quiet_portaudio_decoder_destroy(quiet_portaudio_decoder *d);

int Pa_GetVersion( void );
const char* Pa_GetVersionText( void );

//#define paMakeVersionNumber(major, minor, subminor) \
//    (((major)&0xFF)<<16 | ((minor)&0xFF)<<8 | ((subminor)&0xFF))

typedef struct PaVersionInfo {
    int versionMajor;
    int versionMinor;
    int versionSubMinor;
    const char *versionControlRevision;
    const char *versionText;
} PaVersionInfo;

//const PaVersionInfo* Pa_GetVersionInfo( void );

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

const char *Pa_GetErrorText( PaError errorCode );
PaError Pa_Initialize( void );
PaError Pa_Terminate( void );
typedef int PaDeviceIndex;
#define paNoDevice -1
#define paUseHostApiSpecificDeviceSpecification -2
typedef int PaHostApiIndex;
PaHostApiIndex Pa_GetHostApiCount( void );
PaHostApiIndex Pa_GetDefaultHostApi( void );

typedef enum PaHostApiTypeId
{
    paInDevelopment=0,
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

typedef struct PaHostApiInfo
{
    int structVersion;
    PaHostApiTypeId type;
    const char *name;
    int deviceCount;
    PaDeviceIndex defaultInputDevice;
    PaDeviceIndex defaultOutputDevice;
} PaHostApiInfo;

const PaHostApiInfo * Pa_GetHostApiInfo( PaHostApiIndex hostApi );
PaHostApiIndex Pa_HostApiTypeIdToHostApiIndex( PaHostApiTypeId type );
PaDeviceIndex Pa_HostApiDeviceIndexToDeviceIndex( PaHostApiIndex hostApi, int hostApiDeviceIndex );

typedef struct PaHostErrorInfo{
    PaHostApiTypeId hostApiType;
    long errorCode;
    const char *errorText;
} PaHostErrorInfo;

const PaHostErrorInfo* Pa_GetLastHostErrorInfo( void );
PaDeviceIndex Pa_GetDeviceCount( void );
PaDeviceIndex Pa_GetDefaultInputDevice( void );
PaDeviceIndex Pa_GetDefaultOutputDevice( void );
typedef double PaTime;
typedef unsigned long PaSampleFormat;

#define paFloat32        0x00000001
#define paInt32          0x00000002
#define paInt24          0x00000004
#define paInt16          0x00000008
#define paInt8           0x00000010
#define paUInt8          0x00000020
#define paCustomFormat   0x00010000

#define paNonInterleaved 0x80000000
typedef struct PaDeviceInfo
{
    int structVersion;
    const char *name;
    PaHostApiIndex hostApi;
    int maxInputChannels;
    int maxOutputChannels;
    PaTime defaultLowInputLatency;
    PaTime defaultLowOutputLatency;
    PaTime defaultHighInputLatency;
    PaTime defaultHighOutputLatency;
    double defaultSampleRate;
} PaDeviceInfo;

const PaDeviceInfo* Pa_GetDeviceInfo( PaDeviceIndex device );

typedef struct PaStreamParameters
{
    PaDeviceIndex device;
    int channelCount;
    PaSampleFormat sampleFormat;
    PaTime suggestedLatency;
    void *hostApiSpecificStreamInfo;
} PaStreamParameters;

#define paFormatIsSupported 0

PaError Pa_IsFormatSupported( const PaStreamParameters *inputParameters,
                              const PaStreamParameters *outputParameters,
                              double sampleRate );

typedef void PaStream;

#define paFramesPerBufferUnspecified  0

typedef unsigned long PaStreamFlags;
#define   paNoFlag          0
#define   paClipOff         0x00000001
#define   paDitherOff       0x00000002
#define   paNeverDropInput  0x00000004
#define   paPrimeOutputBuffersUsingStreamCallback 0x00000008
#define   paPlatformSpecificFlags 0xFFFF0000
typedef struct PaStreamCallbackTimeInfo{
    PaTime inputBufferAdcTime;
    PaTime currentTime;
    PaTime outputBufferDacTime;
} PaStreamCallbackTimeInfo;

typedef unsigned long PaStreamCallbackFlags;
#define paInputUnderflow   0x00000001
#define paInputOverflow    0x00000002
#define paOutputUnderflow  0x00000004
#define paOutputOverflow   0x00000008
#define paPrimingOutput    0x00000010
typedef enum PaStreamCallbackResult
{
    paContinue=0,
    paComplete=1,
    paAbort=2
} PaStreamCallbackResult;

typedef int PaStreamCallback(
    const void *input, void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData );

PaError Pa_OpenStream( PaStream** stream,
                       const PaStreamParameters *inputParameters,
                       const PaStreamParameters *outputParameters,
                       double sampleRate,
                       unsigned long framesPerBuffer,
                       PaStreamFlags streamFlags,
                       PaStreamCallback *streamCallback,
                       void *userData );

PaError Pa_OpenDefaultStream( PaStream** stream,
                              int numInputChannels,
                              int numOutputChannels,
                              PaSampleFormat sampleFormat,
                              double sampleRate,
                              unsigned long framesPerBuffer,
                              PaStreamCallback *streamCallback,
                              void *userData );

PaError Pa_CloseStream( PaStream *stream );

typedef void PaStreamFinishedCallback( void *userData );

PaError Pa_SetStreamFinishedCallback( PaStream *stream, PaStreamFinishedCallback* streamFinishedCallback );

PaError Pa_StartStream( PaStream *stream );

PaError Pa_StopStream( PaStream *stream );

PaError Pa_AbortStream( PaStream *stream );

PaError Pa_IsStreamStopped( PaStream *stream );

PaError Pa_IsStreamActive( PaStream *stream );

typedef struct PaStreamInfo
{

    int structVersion;
    PaTime inputLatency;
    PaTime outputLatency;
    double sampleRate;

} PaStreamInfo;

const PaStreamInfo* Pa_GetStreamInfo( PaStream *stream );

PaTime Pa_GetStreamTime( PaStream *stream );

double Pa_GetStreamCpuLoad( PaStream* stream );

PaError Pa_ReadStream( PaStream* stream,
                       void *buffer,
                       unsigned long frames );

PaError Pa_WriteStream( PaStream* stream,
                        const void *buffer,
                        unsigned long frames );

signed long Pa_GetStreamReadAvailable( PaStream* stream );

signed long Pa_GetStreamWriteAvailable( PaStream* stream );
PaError Pa_GetSampleSize( PaSampleFormat format );

void Pa_Sleep( long msec );

