#include <com/com.h>
#include <core/core.h>
#include <stdlib.h>
#include <tracing/tracing.h>

#include <interfaces/IDolby.h>
#include <interfaces/IPlayerInfo.h>
#include <playerinfo.h>
#include <thread>

namespace WPEFramework {
class PlayerInfo {
private:
    typedef std::map<playerinfo_dolby_audio_updated_cb, void*> Callbacks;

    class Notification : public Exchange::Dolby::IOutput::INotification {
    public:
        Notification() = delete;
        Notification(const Notification&) = delete;
        Notification& operator=(const Notification&) = delete;

        Notification(PlayerInfo* parent)
            : _parent(*parent)
        {
        }

        void AudioModeChanged(const Exchange::Dolby::IOutput::SoundModes mode, const bool enabled) override
        {
            _parent.Updated(mode, enabled);
        }

        BEGIN_INTERFACE_MAP(Notification)
        INTERFACE_ENTRY(Exchange::Dolby::IOutput::INotification)
        END_INTERFACE_MAP

    private:
        PlayerInfo& _parent;
    };

    const string _name;
    Exchange::IPlayerProperties* _playerConnection;
    Exchange::Dolby::IOutput* _dolby;
    Core::Sink<Notification> _notification;
    Callbacks _callbacks;

public:
    PlayerInfo() = delete;
    PlayerInfo(const PlayerInfo&) = delete;
    PlayerInfo& operator=(const PlayerInfo&) = delete;

#ifdef __WINDOWS__
#pragma warning(disable : 4355)
#endif
    PlayerInfo(const string& playerName, Exchange::IPlayerProperties* interface)
        : _name(playerName)
        , _playerConnection(interface)
        , _dolby(interface != nullptr ? interface->QueryInterface<Exchange::Dolby::IOutput>() : nullptr)
        , _notification(this)
        , _callbacks()
    {
        ASSERT(_playerConnection != nullptr);
        _playerConnection->AddRef();
    }
#ifdef __WINDOWS__
#pragma warning(default : 4355)
#endif

    ~PlayerInfo()
    {
        if (_playerConnection != nullptr) {
            _playerConnection->Release();
        }
        if (_dolby != nullptr) {
            _dolby->Release();
        }
    }

    const string& Name() const
    {
        return _name;
    }

    void Updated(const Exchange::Dolby::IOutput::SoundModes mode, const bool enabled)
    {
        Callbacks::iterator index(_callbacks.begin());

        while (index != _callbacks.end()) {
            index->first(reinterpret_cast<playerinfo_type*>(this), index->second);
            index++;
        }
    }
    void Register(playerinfo_dolby_audio_updated_cb callback, void* userdata)
    {
        Callbacks::iterator index(_callbacks.find(callback));

        if (index == _callbacks.end()) {
            _callbacks.emplace(std::piecewise_construct,
                std::forward_as_tuple(callback),
                std::forward_as_tuple(userdata));
        }
    }
    void Unregister(playerinfo_dolby_audio_updated_cb callback)
    {
        Callbacks::iterator index(_callbacks.find(callback));

        if (index != _callbacks.end()) {
            _callbacks.erase(index);
        }
    }

    uint32_t IsAudioEquivalenceEnabled(bool& outIsEnabled) const
    {
        ASSERT(_playerConnection != nullptr);

        return _playerConnection->IsAudioEquivalenceEnabled(outIsEnabled);
    }

    uint32_t PlaybackResolution(Exchange::IPlayerProperties::PlaybackResolution& outResolution) const
    {
        ASSERT(_playerConnection != nullptr);

        return _playerConnection->Resolution(outResolution);
    }

    int8_t VideoCodecs(playerinfo_videocodec_t array[], const uint8_t length) const
    {
        Exchange::IPlayerProperties::IVideoCodecIterator* videoCodecs;

        int8_t value = 0;

        ASSERT(_playerConnection != nullptr);

        if (_playerConnection->VideoCodecs(videoCodecs) == Core::ERROR_NONE && videoCodecs != nullptr) {

            Exchange::IPlayerProperties::VideoCodec codec;

            uint8_t numberOfCodecs = 0;
            const uint8_t newArraySize = 50;
            playerinfo_videocodec_t newArray[newArraySize];

            while (videoCodecs->Next(codec) && numberOfCodecs < newArraySize) {
                switch (codec) {
                case Exchange::IPlayerProperties::VideoCodec::VIDEO_UNDEFINED:
                    newArray[numberOfCodecs] = PLAYERINFO_VIDEO_UNDEFINED;
                    break;
                case Exchange::IPlayerProperties::VideoCodec::VIDEO_H263:
                    newArray[numberOfCodecs] = PLAYERINFO_VIDEO_H263;
                    break;
                case Exchange::IPlayerProperties::VideoCodec::VIDEO_H264:
                    newArray[numberOfCodecs] = PLAYERINFO_VIDEO_H264;
                    break;
                case Exchange::IPlayerProperties::VideoCodec::VIDEO_H265:
                    newArray[numberOfCodecs] = PLAYERINFO_VIDEO_H265;
                    break;
                case Exchange::IPlayerProperties::VideoCodec::VIDEO_H265_10:
                    newArray[numberOfCodecs] = PLAYERINFO_VIDEO_H265_10;
                    break;
                case Exchange::IPlayerProperties::VideoCodec::VIDEO_MPEG:
                    newArray[numberOfCodecs] = PLAYERINFO_VIDEO_MPEG;
                    break;
                case Exchange::IPlayerProperties::VideoCodec::VIDEO_VP8:
                    newArray[numberOfCodecs] = PLAYERINFO_VIDEO_VP8;
                    break;
                case Exchange::IPlayerProperties::VideoCodec::VIDEO_VP9:
                    newArray[numberOfCodecs] = PLAYERINFO_VIDEO_VP9;
                    break;
                case Exchange::IPlayerProperties::VideoCodec::VIDEO_VP10:
                    newArray[numberOfCodecs] = PLAYERINFO_VIDEO_VP10;
                    break;
                default:
                    fprintf(stderr, "New video codec in the interface, not handled in client library!\n");
                    ASSERT(false && "Invalid enum");
                    newArray[numberOfCodecs] = PLAYERINFO_VIDEO_UNDEFINED;
                    break;
                }
                ++numberOfCodecs;
            }
            if (numberOfCodecs < length) {
                value = numberOfCodecs;
                ::memcpy(array, newArray, numberOfCodecs * sizeof(playerinfo_videocodec_t));
            } else {
                value = -numberOfCodecs;
            }
        }

        return value;
    }

    int8_t AudioCodecs(playerinfo_audiocodec_t array[], const uint8_t length) const
    {
        Exchange::IPlayerProperties::IAudioCodecIterator* audioCodecs;
        int8_t value = 0;

        ASSERT(_playerConnection != nullptr);

        if (_playerConnection->AudioCodecs(audioCodecs) == Core::ERROR_NONE && audioCodecs != nullptr) {

            Exchange::IPlayerProperties::AudioCodec codec;

            uint8_t numberOfCodecs = 0;
            const uint8_t newArraySize = 50;
            playerinfo_audiocodec_t newArray[newArraySize];

            while (audioCodecs->Next(codec) && numberOfCodecs < newArraySize) {
                switch (codec) {
                case Exchange::IPlayerProperties::AudioCodec::AUDIO_UNDEFINED:
                    newArray[numberOfCodecs] = PLAYERINFO_AUDIO_UNDEFINED;
                    break;
                case Exchange::IPlayerProperties::AudioCodec::AUDIO_AAC:
                    newArray[numberOfCodecs] = PLAYERINFO_AUDIO_AAC;
                    break;
                case Exchange::IPlayerProperties::AudioCodec::AUDIO_AC3:
                    newArray[numberOfCodecs] = PLAYERINFO_AUDIO_AC3;
                    break;
                case Exchange::IPlayerProperties::AudioCodec::AUDIO_AC3_PLUS:
                    newArray[numberOfCodecs] = PLAYERINFO_AUDIO_AC3_PLUS;
                    break;
                case Exchange::IPlayerProperties::AudioCodec::AUDIO_DTS:
                    newArray[numberOfCodecs] = PLAYERINFO_AUDIO_DTS;
                    break;
                case Exchange::IPlayerProperties::AudioCodec::AUDIO_MPEG1:
                    newArray[numberOfCodecs] = PLAYERINFO_AUDIO_MPEG1;
                    break;
                case Exchange::IPlayerProperties::AudioCodec::AUDIO_MPEG2:
                    newArray[numberOfCodecs] = PLAYERINFO_AUDIO_MPEG2;
                    break;
                case Exchange::IPlayerProperties::AudioCodec::AUDIO_MPEG3:
                    newArray[numberOfCodecs] = PLAYERINFO_AUDIO_MPEG3;
                    break;
                case Exchange::IPlayerProperties::AudioCodec::AUDIO_MPEG4:
                    newArray[numberOfCodecs] = PLAYERINFO_AUDIO_MPEG4;
                    break;
                case Exchange::IPlayerProperties::AudioCodec::AUDIO_OPUS:
                    newArray[numberOfCodecs] = PLAYERINFO_AUDIO_OPUS;
                    break;
                case Exchange::IPlayerProperties::AudioCodec::AUDIO_VORBIS_OGG:
                    newArray[numberOfCodecs] = PLAYERINFO_AUDIO_VORBIS_OGG;
                    break;
                case Exchange::IPlayerProperties::AudioCodec::AUDIO_WAV:
                    newArray[numberOfCodecs] = PLAYERINFO_AUDIO_WAV;
                    break;
                default:
                    fprintf(stderr, "New audio codec in the interface, not handled in client library!\n");
                    ASSERT(false && "Invalid enum");
                    newArray[numberOfCodecs] = PLAYERINFO_AUDIO_UNDEFINED;
                    break;
                }
                ++numberOfCodecs;
            }
            if (numberOfCodecs < length) {
                value = numberOfCodecs;
                ::memcpy(array, newArray, numberOfCodecs * sizeof(playerinfo_audiocodec_t));
            } else {
                value = -numberOfCodecs;
            }
        }

        return value;
    }

    bool IsAtmosMetadataSupported() const
    {
        if (_dolby != nullptr) {
            bool isSupported = false;
            if (_dolby->AtmosMetadata(isSupported) != Core::ERROR_NONE) {
                return false;
            }
            return isSupported;
        }
        return false;
    }

    uint32_t DolbySoundMode(Exchange::Dolby::IOutput::SoundModes& mode) const
    {
        ASSERT(_dolby != nullptr);

        return _dolby->SoundMode(mode);
    }

    uint32_t EnableAtmosOutput(const bool enabled)
    {
        ASSERT(_dolby != nullptr);

        return _dolby->EnableAtmosOutput(enabled);
    }

    uint32_t SetDolbyMode(const Exchange::Dolby::IOutput::Type& mode)
    {
        ASSERT(_dolby != nullptr);

        return _dolby->Mode(mode);
    }

    uint32_t GetDolbyMode(Exchange::Dolby::IOutput::Type& mode) const
    {
        ASSERT(_dolby != nullptr);

        const WPEFramework::Exchange::Dolby::IOutput* constDolby = _dolby;
        return constDolby->Mode(mode);
    }
};

//PROXY

class ReconnectionProxy {
private:
    ReconnectionProxy()
        : _engine(Core::ProxyType<RPC::InvokeServerType<1, 0, 8>>::Create())
        , _comChannel(Core::ProxyType<RPC::CommunicatorClient>::Create(Connector(), Core::ProxyType<Core::IIPCServer>(_engine)))
        , _notification(this)
        , _systemInterface(nullptr)
        , _playerInfo(nullptr)
        , _callsign("PlayerInfo")
        , _timeToEnd(false)
        , _event(false, true)
    {
        _adminLock.Lock();

        ASSERT(_engine != nullptr);
        ASSERT(_comChannel != nullptr);
        _engine->Announcements(_comChannel->Announcement());

        _systemInterface = _comChannel->Open<PluginHost::IShell>(string());
        _systemInterface->AddRef();

        ASSERT(_systemInterface != nullptr);
        if (_systemInterface != nullptr) {
            _thread = std::thread(&ReconnectionProxy::Reinstantiate, this);
            _notification.Initialize(_systemInterface);

            bool isCreated = CreateInstance();
            ASSERT(isCreated == true);
        }
        _adminLock.Unlock();
    }

    ~ReconnectionProxy()
    {
        _notification.Deinitialize();
        if (_systemInterface != nullptr) {
            _systemInterface->Release();
        }

        _timeToEnd = true;

        _event.SetEvent();
        if (_thread.joinable()) {
            _thread.join();
        }

        delete _playerInfo;
        if (_comChannel.IsValid()) {
            _comChannel->Close(1000);
        }
    }

    ReconnectionProxy(const ReconnectionProxy&) = delete;
    ReconnectionProxy& operator=(const ReconnectionProxy&) = delete;

    static Core::NodeId Connector()
    {
        const TCHAR* comPath = ::getenv(_T("COMMUNICATOR_PATH"));

        if (comPath == nullptr) {
#ifdef __WINDOWS__
            comPath = _T("127.0.0.1:62000");
#else
            comPath = _T("/tmp/communicator");
#endif
        }

        return Core::NodeId(comPath);
    }

    static ReconnectionProxy* _instance;

private:
    void StateChange(PluginHost::IShell* plugin)
    {
        _lock.Lock();

        PluginHost::IShell::state state = plugin->State();
        if (_callsign == plugin->Callsign()) {
            if (state == PluginHost::IShell::ACTIVATED) {
                _event.SetEvent();
                fprintf(stderr, "ACTIVATING\n");

            } else if (state == PluginHost::IShell::DEACTIVATION) {
                fprintf(stderr, "DEACTIVATING\n");
                delete _playerInfo;
                _playerInfo = nullptr;
            }
        }

        _lock.Unlock();
    }
    bool CreateInstance()
    {
        ASSERT(_systemInterface != nullptr);
        ASSERT(_playerInfo == nullptr);
        bool isCreated = false;

        if (_playerInfo == nullptr) {
            Exchange::IPlayerProperties* playerInfoInterface = _systemInterface->QueryInterfaceByCallsign<Exchange::IPlayerProperties>(_callsign);

            ASSERT(playerInfoInterface != nullptr);
            if (playerInfoInterface != nullptr) {
                _playerInfo = new PlayerInfo(_callsign, playerInfoInterface);
                playerInfoInterface->Release();

                if (_playerInfo != nullptr) {
                    isCreated = true;
                }
            }
        }
        return isCreated;
    }

    void Reinstantiate()
    {
        while (true) {

            _event.Lock();
            _event.ResetEvent();

            if (_timeToEnd) {
                return;
            }
            CreateInstance();
        }
    }

private:
    class Notification : protected PluginHost::IPlugin::INotification {
    public:
        Notification() = delete;
        Notification(const Notification&) = delete;
        Notification& operator=(const Notification&) = delete;
        ~Notification() = default;

        explicit Notification(ReconnectionProxy* parent)
            : _parent(*parent)
            , _client(nullptr)
            , _isRegistered(false)
        {
            ASSERT(parent != nullptr);
        }

        void Initialize(PluginHost::IShell* client)
        {
            ASSERT(client != nullptr);
            _client = client;
            _client->AddRef();
            _client->Register(this);
            _isRegistered = true;
        }
        void Deinitialize()
        {
            ASSERT(_client != nullptr);
            if (_client != nullptr) {
                _client->Unregister(this);
                _isRegistered = false;
                _client->Release();
                _client = nullptr;
            }
        }

        void StateChange(PluginHost::IShell* plugin) override
        {
            ASSERT(plugin != nullptr);

            if (_isRegistered) {
                _parent.StateChange(plugin);
            }
        }

        BEGIN_INTERFACE_MAP(Notification)
        INTERFACE_ENTRY(PluginHost::IPlugin::INotification)
        END_INTERFACE_MAP

    private:
        ReconnectionProxy& _parent;
        bool _isRegistered;
        PluginHost::IShell* _client;
    };

public:
    //TODO what if construction fails
    static ReconnectionProxy* Instance()
    {
        if (_instance == nullptr) {
            _instance = new ReconnectionProxy();
        }
        return _instance;
    }
    void Delete()
    {
        delete _instance;
    }

public:
    uint32_t IsAudioEquivalenceEnabled(bool& outIsEnabled) const
    {
        if (_playerInfo != nullptr) {
            fprintf(stderr, "NOT NULL\n");

            return _playerInfo->IsAudioEquivalenceEnabled(outIsEnabled);
        }
        return Core::ERROR_UNAVAILABLE;
    }

private:
    Core::Sink<Notification> _notification;

    Core::ProxyType<RPC::InvokeServerType<1, 0, 8>> _engine;
    Core::ProxyType<RPC::CommunicatorClient> _comChannel;

    PluginHost::IShell* _systemInterface;
    PlayerInfo* _playerInfo;
    std::string _callsign;

    mutable Core::CriticalSection _adminLock;
    bool _timeToEnd;
    std::thread _thread;
    Core::CriticalSection _lock;
    Core::Event _event;
};
ReconnectionProxy* ReconnectionProxy::_instance;

//!PROXY
} //namespace WPEFramework

using namespace WPEFramework;
extern "C" {

struct playerinfo_type* playerinfo_instance(const char name[])
{
    if (name != NULL) {
        return reinterpret_cast<playerinfo_type*>(ReconnectionProxy::Instance());
    }
    return NULL;
}

void playerinfo_register(struct playerinfo_type* instance, playerinfo_dolby_audio_updated_cb callback, void* userdata)
{
    if (instance != NULL) {
        reinterpret_cast<PlayerInfo*>(instance)->Register(callback, userdata);
    }
}

void playerinfo_unregister(struct playerinfo_type* instance, playerinfo_dolby_audio_updated_cb callback)
{
    if (instance != NULL) {
        reinterpret_cast<PlayerInfo*>(instance)->Unregister(callback);
    }
}

void playerinfo_release(struct playerinfo_type* instance)
{
    if (instance != NULL) {
        reinterpret_cast<ReconnectionProxy*>(instance)->Delete();
    }
}

uint32_t playerinfo_playback_resolution(struct playerinfo_type* instance, playerinfo_playback_resolution_t* resolution)
{
    if (instance != NULL && resolution != NULL) {
        Exchange::IPlayerProperties::PlaybackResolution value = Exchange::IPlayerProperties::PlaybackResolution::RESOLUTION_UNKNOWN;

        if (reinterpret_cast<PlayerInfo*>(instance)->PlaybackResolution(value) == Core::ERROR_NONE) {
            switch (value) {
            case Exchange::IPlayerProperties::PlaybackResolution::RESOLUTION_UNKNOWN:
                *resolution = PLAYERINFO_RESOLUTION_UNKNOWN;
                break;
            case Exchange::IPlayerProperties::PlaybackResolution::RESOLUTION_480I:
                *resolution = PLAYERINFO_RESOLUTION_480I;
                break;
            case Exchange::IPlayerProperties::PlaybackResolution::RESOLUTION_480P:
                *resolution = PLAYERINFO_RESOLUTION_480P;
                break;
            case Exchange::IPlayerProperties::PlaybackResolution::RESOLUTION_576I:
                *resolution = PLAYERINFO_RESOLUTION_576I;
                break;
            case Exchange::IPlayerProperties::PlaybackResolution::RESOLUTION_576P:
                *resolution = PLAYERINFO_RESOLUTION_576P;
                break;
            case Exchange::IPlayerProperties::PlaybackResolution::RESOLUTION_720P:
                *resolution = PLAYERINFO_RESOLUTION_720P;
                break;
            case Exchange::IPlayerProperties::PlaybackResolution::RESOLUTION_1080I:
                *resolution = PLAYERINFO_RESOLUTION_1080I;
                break;
            case Exchange::IPlayerProperties::PlaybackResolution::RESOLUTION_1080P:
                *resolution = PLAYERINFO_RESOLUTION_1080P;
                break;
            case Exchange::IPlayerProperties::PlaybackResolution::RESOLUTION_2160P30:
                *resolution = PLAYERINFO_RESOLUTION_2160P30;
                break;
            case Exchange::IPlayerProperties::PlaybackResolution::RESOLUTION_2160P60:
                *resolution = PLAYERINFO_RESOLUTION_2160P60;
                break;
            default:
                fprintf(stderr, "New resolution in the interface, not handled in client library!\n");
                ASSERT(false && "Invalid enum");
                *resolution = PLAYERINFO_RESOLUTION_UNKNOWN;
                return Core::ERROR_UNKNOWN_KEY;
            }
            return Core::ERROR_NONE;
        }
    }
    return Core::ERROR_UNAVAILABLE;
}

uint32_t playerinfo_is_audio_equivalence_enabled(struct playerinfo_type* instance, bool* is_enabled)
{
    if (instance != NULL && is_enabled != NULL) {
        return reinterpret_cast<ReconnectionProxy*>(instance)->IsAudioEquivalenceEnabled(*is_enabled);
    }
    return Core::ERROR_UNAVAILABLE;
}

int8_t playerinfo_video_codecs(struct playerinfo_type* instance, playerinfo_videocodec_t array[], const uint8_t length)
{
    if (instance != NULL && array != NULL) {
        return reinterpret_cast<PlayerInfo*>(instance)->VideoCodecs(array, length);
    }
    return 0;
}

int8_t playerinfo_audio_codecs(struct playerinfo_type* instance, playerinfo_audiocodec_t array[], const uint8_t length)
{
    if (instance != NULL && array != NULL) {
        return reinterpret_cast<PlayerInfo*>(instance)->AudioCodecs(array, length);
    }
    return 0;
}

bool playerinfo_is_dolby_atmos_supported(struct playerinfo_type* instance)
{
    if (instance != NULL) {
        return reinterpret_cast<PlayerInfo*>(instance)->IsAtmosMetadataSupported();
    }
    return false;
}

uint32_t playerinfo_set_dolby_sound_mode(struct playerinfo_type* instance, playerinfo_dolby_sound_mode_t* sound_mode)
{
    if (instance != NULL && sound_mode != NULL) {
        Exchange::Dolby::IOutput::SoundModes value = Exchange::Dolby::IOutput::SoundModes::UNKNOWN;
        if (reinterpret_cast<PlayerInfo*>(instance)->DolbySoundMode(value) == Core::ERROR_NONE) {
            switch (value) {
            case Exchange::Dolby::IOutput::SoundModes::UNKNOWN:
                *sound_mode = PLAYERINFO_DOLBY_SOUND_UNKNOWN;
                break;
            case Exchange::Dolby::IOutput::SoundModes::MONO:
                *sound_mode = PLAYERINFO_DOLBY_SOUND_MONO;
                break;
            case Exchange::Dolby::IOutput::SoundModes::STEREO:
                *sound_mode = PLAYERINFO_DOLBY_SOUND_STEREO;
                break;
            case Exchange::Dolby::IOutput::SoundModes::SURROUND:
                *sound_mode = PLAYERINFO_DOLBY_SOUND_SURROUND;
                break;
            case Exchange::Dolby::IOutput::SoundModes::PASSTHRU:
                *sound_mode = PLAYERINFO_DOLBY_SOUND_PASSTHRU;
                break;
            default:
                fprintf(stderr, "New dolby sound mode in the interface, not handled in client library!\n");
                ASSERT(false && "Invalid enum");
                *sound_mode = PLAYERINFO_DOLBY_SOUND_UNKNOWN;
                return Core::ERROR_UNKNOWN_KEY;
                break;
            }
            return Core::ERROR_NONE;
        }
    }
    return Core::ERROR_UNAVAILABLE;
}
uint32_t playerinfo_enable_atmos_output(struct playerinfo_type* instance, const bool is_enabled)
{
    if (instance != NULL) {
        return reinterpret_cast<PlayerInfo*>(instance)->EnableAtmosOutput(is_enabled);
    }
    return Core::ERROR_UNAVAILABLE;
}

uint32_t playerinfo_set_dolby_mode(struct playerinfo_type* instance, const playerinfo_dolby_mode_t mode)
{
    if (instance != NULL) {
        switch (mode) {
        case PLAYERINFO_DOLBY_MODE_AUTO:
            return reinterpret_cast<PlayerInfo*>(instance)->SetDolbyMode(Exchange::Dolby::IOutput::Type::AUTO);
        case PLAYERINFO_DOLBY_MODE_DIGITAL_PCM:
            return reinterpret_cast<PlayerInfo*>(instance)->SetDolbyMode(Exchange::Dolby::IOutput::Type::DIGITAL_PCM);
        case PLAYERINFO_DOLBY_MODE_DIGITAL_AC3:
            return reinterpret_cast<PlayerInfo*>(instance)->SetDolbyMode(Exchange::Dolby::IOutput::Type::DIGITAL_AC3);
        case PLAYERINFO_DOLBY_MODE_DIGITAL_PLUS:
            return reinterpret_cast<PlayerInfo*>(instance)->SetDolbyMode(Exchange::Dolby::IOutput::Type::DIGITAL_PLUS);
        case PLAYERINFO_DOLBY_MODE_MS12:
            return reinterpret_cast<PlayerInfo*>(instance)->SetDolbyMode(Exchange::Dolby::IOutput::Type::MS12);
        default:
            fprintf(stderr, "Unknown enum value, not included in playerinfo_dolby_mode_type?\n");
            return Core::ERROR_UNKNOWN_KEY;
        }
    }
    return Core::ERROR_UNAVAILABLE;
}

uint32_t playerinfo_get_dolby_mode(struct playerinfo_type* instance, playerinfo_dolby_mode_t* mode)
{
    if (instance != NULL && mode != NULL) {

        Exchange::Dolby::IOutput::Type value = Exchange::Dolby::IOutput::Type::AUTO;
        if (reinterpret_cast<PlayerInfo*>(instance)->GetDolbyMode(value) == Core::ERROR_NONE) {
            switch (value) {
            case Exchange::Dolby::IOutput::Type::AUTO:
                *mode = PLAYERINFO_DOLBY_MODE_AUTO;
                break;
            case Exchange::Dolby::IOutput::Type::DIGITAL_AC3:
                *mode = PLAYERINFO_DOLBY_MODE_DIGITAL_AC3;
                break;
            case Exchange::Dolby::IOutput::Type::DIGITAL_PCM:
                *mode = PLAYERINFO_DOLBY_MODE_DIGITAL_PCM;
                break;
            case Exchange::Dolby::IOutput::Type::DIGITAL_PLUS:
                *mode = PLAYERINFO_DOLBY_MODE_DIGITAL_PLUS;
                break;
            case Exchange::Dolby::IOutput::Type::MS12:
                *mode = PLAYERINFO_DOLBY_MODE_MS12;
                break;
            default:
                fprintf(stderr, "New dolby mode in the interface, not handled in client library!\n");
                ASSERT(false && "Invalid enum");
                *mode = PLAYERINFO_DOLBY_MODE_AUTO;
                return Core::ERROR_UNKNOWN_KEY;
            }
            return Core::ERROR_NONE;
        }
    }
    return Core::ERROR_UNAVAILABLE;
}
}