/**
 * @file BotBase.hpp
 * @brief Клиент для работы с VK Bot Long Poll API.
 *
 * @version 0.1.0
 */

#pragma once

#include "vkbot/Types.hpp"
#include <future>
#include <string>
#include <atomic>

#include <vkbot/ClientBase.hpp>

namespace vk::bot {

inline constexpr std::string_view kDefaultTimeWait = "25";

class BotBase final : public base::ClientBase {
public:
    // -----------------------------------------------------------------------
    // Методы, доступные боту (подмножество VK API)
    // -----------------------------------------------------------------------
    enum class Method {
        DeleteComment,
        RestoreComment,
        AddAddress,
        DeleteAddress,
        DisableOnline,
        EditAddress,
        EnableOnline,
        GetBanned,
        GetLongPollServer,
        GetLongPollSettings,
        GetMembers,
        GetOnlineStatus,
        GetTokenPermissions,
        IsMember,
        SetLongPollSettings,
        SetSettings,
        MarkAsRead, 
        CreateChat,
        DeleteMessage,
        DeleteChatPhoto,
        DeleteConversation,
        EditMessage,
        EditChat,
        GetByConversationMessageId,
        GetByMessageId,
        GetConversationMembers,
        GetConversations,
        GetConversationById,
        GetHistory,
        GetInviteLink,
        PinMessage,
        RemoveChatUser,
        RestoreMessage,
        SearchMessage,
        SendMessage,
        UnpinMessage,
        GetUser,
        CloseComments,
        CreateComment,
        OpenComments,
    };

    // -----------------------------------------------------------------------
    // События Long Poll
    // -----------------------------------------------------------------------
    enum class Event {
        MessageNew,
        MessageReply,
        MessageAllow,
        MessageDeny,
        PhotoNew,
        AudioNew,
        VideoNew,
        WallReplyNew,
        WallReplyEdit,
        WallReplyDelete,
        WallPostNew,
        WallRepost,
        BoardPostNew,
        BoardPostEdit,
        BoardPostDelete,
        BoardPostRestore,
        PhotoCommentNew,
        PhotoCommentEdit,
        PhotoCommentDelete,
        PhotoCommentRestore,
        VideoCommentNew,
        VideoCommentEdit,
        VideoCommentDelete,
        VideoCommentRestore,
        MarketCommentNew,
        MarketCommentEdit,
        MarketCommentDelete,
        MarketCommentRestore,
        PollVoteNew,
        GroupJoin,
        GroupLeave,
        UserBlock,
        UserUnblock,
        GroupChangeSettings,
        GroupChangePhoto,
        GroupOfficersEdit,
        Unknown,
    };

    // -----------------------------------------------------------------------
    struct EventData {
        Event          type;
        base::JsonType payload;
    };

    // -----------------------------------------------------------------------
    /**
     * @param group_id  ID группы (без минуса).
     * @param time_wait Таймаут ожидания события на Long Poll сервере (сек).
     */
    explicit BotBase(std::string group_id,
                     std::string time_wait = std::string(kDefaultTimeWait));

    ~BotBase() override = default;

    // -----------------------------------------------------------------------
    // ClientBase interface
    // -----------------------------------------------------------------------

    /**
     * @brief Авторизация по токену сообщества.
     * @throws ex::AlreadyConnectedException если уже авторизован.
     * @throws ex::EmptyArgumentException    если токен пустой.
     * @throws ex::AuthFailedException       при ошибке сервера.
     */
    bool auth(const std::string& access_token) override;

    /**
     * @brief Отправить запрос по строковому имени метода.
     * @throws ex::NotConnectedException если не авторизован.
     */
    base::JsonType send_request(const std::string& method,
                                const base::JsonType& params) override;

    // -----------------------------------------------------------------------
    // Bot-specific API
    // -----------------------------------------------------------------------


    /**
    * @brief Блокирует поток до поступления события от Long Poll сервера.
    *
    * @param ec  Устанавливается в boost::asio::error::operation_aborted
    *            при прерывании через interrupt(). При успехе — очищается.
    *            При прерывании возвращает EventData{Event::Unknown, {}}.
    * @throws ex::NotConnectedException если не авторизован.
    */
    [[nodiscard]] EventData wait_for_event(boost::system::error_code& ec);

    /**
     * @brief Блокирует поток до поступления события от Long Poll сервера.
     * @return EventData с типом и JSON-телом события.
     * @throws ex::NotConnectedException если не авторизован.
     */
    [[nodiscard]] EventData wait_for_event();

    /**
    * @brief Прерывает текущий (или следующий) вызов wait_for_event().
    *
    * Потокобезопасно. Вызов из любого потока немедленно будит заблокированный
    * wait_for_event(): оригинальный overload бросает ex::InterruptedException,
    * overload с error_code устанавливает boost::asio::error::operation_aborted.
    *
    * Для возобновления работы вызвать reset_interrupt().
    */
    void interrupt();

    /**
    * @brief Сбрасывает флаг прерывания после interrupt().
    *
    * Необходимо вызвать перед следующим wait_for_event() чтобы
    * продолжать принимать события.
    */
    void reset_interrupt();

    /**
     * @brief Отправить запрос по enum-методу.
     */
    base::JsonType send_request(Method method, const base::JsonType& params);

    /**
     * @brief Асинхронная версия send_request (enum).
     *
     * Создаёт отдельный HttpClient внутри нового потока — нет гонок,
     * нет утечек (в отличие от оригинала, где asyncCurl не чистился
     * при исключении).
     */
    [[nodiscard]] std::future<base::JsonType>
    send_request_async(Method method, const base::JsonType& params);

    /**
     * @brief Асинхронная версия send_request (строка).
     */
    [[nodiscard]] std::future<base::JsonType>
    send_request_async(const std::string& method, const base::JsonType& params);

    /// Конвертирует enum метода в строку VK API.
    [[nodiscard]] static std::string method_to_string(Method method);

    /// Методы для упрощенного взаимодействия с VK
    
    //base::JsonType send_photo_request();

     /**
     * @brief Отправка файла на сервер
     * @returns Ответ файлового сервера
     */
    base::JsonType send_file_request(const std::string& url,
                                     const std::string& filePath,
                                     const std::string& fieldName);

    /**
    * @brief Отправка данных на сервер как файла.
    * @param url       Адрес загрузки.
    * @param data      Бинарные данные файла.
    * @param filename  Имя файла (для поля filename).
    * @param fieldName Имя поля в multipart.
    * @param mime_type MIME-тип (по умолчанию application/octet-stream).
    * @return Ответ сервера в JSON.
    */
    base::JsonType send_file_request(const std::string& url,
                                 const unsigned char* data,
                                 size_t size,
                                 const std::string& filename,
                                 const std::string& fieldName,
                                 const std::string& mime_type = "application/octet-stream");


protected:
    [[nodiscard]] base::JsonType fill_required_params(const base::JsonType& params) const override;

private:
    [[nodiscard]] static Event parse_event_type(std::string_view type_str) noexcept;
    void refresh_long_poll_server();

    std::string m_group_id;
    std::string m_access_token;
    std::string m_time_wait;

    // Long Poll сессия
    std::string m_server_url;
    std::string m_secret_key;
    std::string m_timestamp;
    std::atomic<bool> m_interrupted{false};
};

} // namespace vk::bot
