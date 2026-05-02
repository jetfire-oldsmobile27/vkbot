/**
 * @file BotBase.cpp
 * @version 0.1.0
 */

#include <vkbot/BotBase.hpp>

#include <cassert>
#include <unordered_map>

static inline bool sv_starts_with(std::string_view s, std::string_view prefix) noexcept {
    return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
}

namespace vk::bot {


BotBase::BotBase(std::string group_id, std::string time_wait)
    : m_group_id(std::move(group_id))
    , m_time_wait(std::move(time_wait))
{

}


bool BotBase::auth(const std::string& access_token) {
    if (m_authorized)         {throw ex::AlreadyConnectedException{};};
    if (access_token.empty()) {throw ex::EmptyArgumentException{};};

    m_access_token = access_token;

    const base::JsonType params = {
        {"access_token", m_access_token},
        {"group_id",     m_group_id},
        {"v",            std::string(base::VKBOT_API_VERSION)},
    };

    const std::string target =
        std::string(base::VKBOT_API_METHOD_PFX) + method_to_string(Method::GetLongPollServer);

    const std::string raw = m_http.post(
        std::string(base::VKBOT_API_HOST), target, params_to_query(params));

    const base::JsonType response = base::JsonType::parse(raw);

    if (response.contains("error")) {
        throw ex::AuthFailedException(response["error"].dump());
    }

    const auto& r = response.at("response");
    m_secret_key  = r.at("key").get<std::string>();
    m_server_url  = r.at("server").get<std::string>();
    m_timestamp   = r.at("ts").get<std::string>();
    m_authorized  = true;

    return true;
}


BotBase::EventData BotBase::wait_for_event() {
    if (!m_authorized) {
        throw ex::NotConnectedException{};
    };

    const std::string query =
        "key="   + m_secret_key +
        "&ts="   + m_timestamp  +
        "&wait=" + m_time_wait  +
        "&act=a_check";

    std::string host, path;
    {
        std::string_view sv = m_server_url;
        if (sv_starts_with(sv, "https://")) {
            sv.remove_prefix(8);
        } else if (sv_starts_with(sv, "http://"))  {
        sv.remove_prefix(7);
        }
        const auto slash = sv.find('/');
        if (slash == std::string_view::npos) {
            host = std::string(sv);
            path = "/";
        } else {
            host = std::string(sv.substr(0, slash));
            path = std::string(sv.substr(slash));
        }
    }
    path += '?' + query;

    const std::string  raw      = m_http.get(host, path);
    base::JsonType     response = base::JsonType::parse(raw);

    if (response.contains("ts")) {
        m_timestamp = response["ts"].get<std::string>();
    }

    if (!response.contains("updates") || !response["updates"].is_array()  || response["updates"].empty()) {
        return {Event::Unknown, base::JsonType{}};
    }

    base::JsonType update    = response["updates"][0];
    const auto     event_type = parse_event_type(update.at("type").get<std::string>());
    return {event_type, std::move(update)};
}


base::JsonType BotBase::send_request(Method method, const base::JsonType& params) {
    return send_request(method_to_string(method), params);
}

base::JsonType BotBase::send_request(const std::string& method, const base::JsonType& params) {
    if (!m_authorized)  {throw ex::NotConnectedException{};};
    if (method.empty()) {throw ex::EmptyArgumentException{};};

    const std::string target = std::string(base::VKBOT_API_METHOD_PFX) + method;
    const auto        full   = fill_required_params(params);
    const std::string raw    = m_http.post(std::string(base::VKBOT_API_HOST), target, 
                                                                        params_to_query(full));

    return base::JsonType::parse(raw);
}


std::future<base::JsonType>
BotBase::send_request_async(Method method, const base::JsonType& params) {
    return send_request_async(method_to_string(method), params);
}

std::future<base::JsonType> BotBase::send_request_async(const std::string& method, const base::JsonType& params) {
    if (!m_authorized)  {throw ex::NotConnectedException{};};
    if (method.empty()) {
        throw ex::EmptyArgumentException{};
    };

    const std::string    target      = std::string(base::VKBOT_API_METHOD_PFX) + method;
    const base::JsonType full_params = fill_required_params(params);
    const std::string    query       = params_to_query(full_params);

    return std::async(std::launch::async,
        [target, query]() -> base::JsonType {
            vk::http::HttpClient local_http;
            const std::string raw = local_http.post(
                std::string(base::VKBOT_API_HOST), target, query);
            return base::JsonType::parse(raw);
        });
}


base::JsonType BotBase::fill_required_params(const base::JsonType& params) const {
    base::JsonType out = params;
    if (!out.contains("access_token")) out["access_token"] = m_access_token;
    if (!out.contains("group_id"))     out["group_id"]     = m_group_id;
    if (!out.contains("v"))            out["v"]            = std::string(base::VKBOT_API_VERSION);
    return out;
}


std::string BotBase::method_to_string(Method method) {
    switch (method) {
    case Method::DeleteComment:              return "board.deleteComment";

    case Method::RestoreComment:             return "board.restoreComment";

    case Method::AddAddress:                 return "groups.addAddress";

    case Method::DeleteAddress:              return "groups.deleteAddress";

    case Method::DisableOnline:              return "groups.disableOnline";

    case Method::EditAddress:                return "groups.editAddress";

    case Method::EnableOnline:               return "groups.enableOnline";

    case Method::GetBanned:                  return "groups.getBanned";

    case Method::GetLongPollServer:          return "groups.getLongPollServer";

    case Method::GetLongPollSettings:        return "groups.getLongPollSettings";

    case Method::GetMembers:                 return "groups.getMembers";

    case Method::GetOnlineStatus:            return "groups.getOnlineStatus";

    case Method::GetTokenPermissions:        return "groups.getTokenPermissions";

    case Method::IsMember:                   return "groups.isMember";

    case Method::SetLongPollSettings:        return "groups.setLongPollSettings";

    case Method::SetSettings:               return "groups.setSettings";

    case Method::CreateChat:                 return "messages.createChat";

    case Method::DeleteMessage:              return "messages.delete";

    case Method::DeleteChatPhoto:            return "messages.deleteChatPhoto";

    case Method::DeleteConversation:         return "messages.deleteConversation";

    case Method::EditMessage:                return "messages.edit";

    case Method::EditChat:                   return "messages.editChat";

    case Method::GetByConversationMessageId: return "messages.getByConversationMessageId";

    case Method::GetByMessageId:             return "messages.getById";

    case Method::GetConversationMembers:     return "messages.getConversationMembers";

    case Method::GetConversations:           return "messages.getConversations";

    case Method::GetConversationById:        return "messages.getConversationById";

    case Method::GetHistory:                 return "messages.getHistory";

    case Method::GetInviteLink:              return "messages.getInviteLink";

    case Method::PinMessage:                 return "messages.pin";

    case Method::RemoveChatUser:             return "messages.removeChatUser";

    case Method::RestoreMessage:             return "messages.restore";

    case Method::SearchMessage:              return "messages.search";

    case Method::SendMessage:                return "messages.send";

    case Method::UnpinMessage:               return "messages.unpin";

    case Method::GetUser:                    return "users.get";

    case Method::CloseComments:              return "wall.closeComments";

    case Method::CreateComment:              return "wall.createComment";

    case Method::OpenComments:               return "wall.openComments";

    }
    return {};
}


BotBase::Event BotBase::parse_event_type(std::string_view type_str) noexcept {
    static const std::unordered_map<std::string, Event> kMap = {
        {"message_new",            Event::MessageNew},

        {"message_reply",          Event::MessageReply},

        {"message_allow",          Event::MessageAllow},

        {"message_deny",           Event::MessageDeny},

        {"photo_new",              Event::PhotoNew},

        {"audio_new",              Event::AudioNew},

        {"video_new",              Event::VideoNew},

        {"wall_reply_new",         Event::WallReplyNew},

        {"wall_reply_edit",        Event::WallReplyEdit},

        {"wall_reply_delete",      Event::WallReplyDelete},

        {"wall_post_new",          Event::WallPostNew},

        {"wall_repost",            Event::WallRepost},

        {"board_post_new",         Event::BoardPostNew},

        {"board_post_edit",        Event::BoardPostEdit},

        {"board_post_delete",      Event::BoardPostDelete},

        {"board_post_restore",     Event::BoardPostRestore},

        {"photo_comment_new",      Event::PhotoCommentNew},

        {"photo_comment_edit",     Event::PhotoCommentEdit},

        {"photo_comment_delete",   Event::PhotoCommentDelete},

        {"photo_comment_restore",  Event::PhotoCommentRestore},

        {"video_comment_new",      Event::VideoCommentNew},

        {"video_comment_edit",     Event::VideoCommentEdit},

        {"video_comment_delete",   Event::VideoCommentDelete},

        {"video_comment_restore",  Event::VideoCommentRestore},

        {"market_comment_new",     Event::MarketCommentNew},

        {"market_comment_edit",    Event::MarketCommentEdit},

        {"market_comment_delete",  Event::MarketCommentDelete},

        {"market_comment_restore", Event::MarketCommentRestore},

        {"poll_vote_new",          Event::PollVoteNew},

        {"group_join",             Event::GroupJoin},

        {"group_leave",            Event::GroupLeave},

        {"user_block",             Event::UserBlock},

        {"user_unblock",           Event::UserUnblock},

        {"group_change_settings",  Event::GroupChangeSettings},

        {"group_change_photo",     Event::GroupChangePhoto},

        {"group_officers_edit",    Event::GroupOfficersEdit},
    };

    if (const auto it = kMap.find(std::string(type_str)); it != kMap.end())
        return it->second;
    return Event::Unknown;
}

} // namespace vk::bot
