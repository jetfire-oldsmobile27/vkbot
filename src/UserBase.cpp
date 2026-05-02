/**
 * @file UserBase.cpp
 * @version 0.1.0
 *
 */

#include <vkbot/UserBase.hpp>

#include <iostream>

namespace vk::user {


UserBase::UserBase(std::string app_id, std::string app_secure_key)
    : m_app_id(std::move(app_id))
    , m_app_secure_key(std::move(app_secure_key))
{
    if (m_app_id.empty() || m_app_secure_key.empty())
        throw ex::EmptyArgumentException{};
}


// ИСПРАВЛЕНИЕ: оригинальная логика была перевёрнута.
// Правило: если ответ НЕ содержит "error" — авторизация успешна.

bool UserBase::auth(const std::string& access_token)
{
    if (access_token.empty()) throw ex::EmptyArgumentException{};
    m_authorized = false;

    const base::JsonType params = {
        {"access_token", access_token},
        {"v",            std::string(base::VKBOT_API_VERSION)},
    };

    const std::string target =
        std::string(base::VKBOT_API_METHOD_PFX) + method_to_string(Method::UsersGet);

    const std::string      raw      = m_http.post(std::string(base::VKBOT_API_HOST), target, params_to_query(params));
    const base::JsonType   response = base::JsonType::parse(raw);

    try {
        if (!response.contains("error")) {
            if (response.contains("response") && response["response"].is_array()
                && !response["response"].empty())
            {
                m_user_id      = std::to_string(
                    response["response"][0].at("id").get<std::int64_t>());
                m_access_token = access_token;
                m_authorized   = true;
            }
        } else {
            m_user_id.clear();
            m_access_token.clear();
            m_authorized = false;
        }
    } catch (const base::JsonType::exception& e) {
        std::cerr << "[VKBOT] JSON parse error in UserBase::auth: " << e.what() << '\n';
    }

    return m_authorized;
}


bool UserBase::auth(const std::string& login, const std::string& password)
{
    if (login.empty() || password.empty()) throw ex::EmptyArgumentException{};
    m_authorized = false;

    // Собираем scope
    std::string scope_str;
    if (!m_scope.empty()) {
        for (const auto& s : m_scope) {
            if (!scope_str.empty()) scope_str += ',';
            scope_str += s;
        }
    }

    base::JsonType params = {
        {"client_id",     m_app_id},
        {"grant_type",    "password"},
        {"client_secret", m_app_secure_key},
        {"scope",         scope_str},
        {"username",      login},
        {"password",      password},
    };

    try {
        const std::string    raw      = m_http.post(std::string(base::VKBOT_OAUTH_HOST),
                                                    "/token",
                                                    params_to_query(params));
        base::JsonType       response = base::JsonType::parse(raw);

        if (response.contains("error")) {
            const std::string err = response["error"].get<std::string>();

            // Капча
            if (err == "need_captcha") {
                const std::string captcha_url =
                    response.at("captcha_img").get<std::string>();
                params["captcha_sid"] = response.at("captcha_sid").get<std::string>();

                std::cout << "Введите капчу (" << captcha_url << "): ";
                std::string captcha_key;
                std::cin  >> captcha_key;
                params["captcha_key"] = captcha_key;

                const std::string raw2 = m_http.post(
                    std::string(base::VKBOT_OAUTH_HOST), "/token", params_to_query(params));
                response = base::JsonType::parse(raw2);
            }

            // 2FA
            if (response.contains("validation_type")) {
                m_authorized = handle_2fa(params, response);
                return m_authorized;
            }
        } else {
            m_access_token = response.at("access_token").get<std::string>();
            m_user_id      = std::to_string(response.at("user_id").get<std::int64_t>());
            m_authorized   = true;
        }
    } catch (const base::JsonType::exception& e) {
        std::cerr << "[VKBOT] JSON error in UserBase::auth(login): " << e.what() << '\n';
    }

    return m_authorized;
}


bool UserBase::handle_2fa(base::JsonType& params, const base::JsonType& error_response)
{
    const auto vtype = parse_validation_type(
        error_response.at("validation_type").get<std::string>());

    if (vtype == ValidationTypes::Unknown) return false;

    params["2fa_supported"] = "1";
    if (vtype == ValidationTypes::TwoFaSms) {
        params["force_sms"] = "1";
        if (error_response.contains("redirect_url")) {
            std::string_view sv = error_response["redirect_url"].get<std::string>();
            if (sv.substr(0, 8) == "https://") sv.remove_prefix(8);
            const auto slash = sv.find('/');
            const std::string rhost = std::string(sv.substr(0, slash));
            const std::string rpath = std::string(sv.substr(slash));
            std::ignore = m_http.get(rhost, rpath);
        }
    }

    std::cout << "Введите код подтверждения: ";
    std::string code;
    std::cin >> code;
    params["code"] = code;

    try {
        const std::string raw = m_http.post(
            std::string(base::VKBOT_OAUTH_HOST), "/token", params_to_query(params));
        const base::JsonType resp = base::JsonType::parse(raw);

        if (!resp.contains("error")) {
            m_access_token = resp.at("access_token").get<std::string>();
            m_user_id      = std::to_string(resp.at("user_id").get<std::int64_t>());
            return true;
        }
    } catch (const base::JsonType::exception& e) {
        std::cerr << "[VKBOT] JSON error in handle_2fa: " << e.what() << '\n';
    }
    return false;
}


base::JsonType UserBase::send_request(Method method, const base::JsonType& params)
{
    return send_request(method_to_string(method), params);
}

base::JsonType UserBase::send_request(const std::string& method,
                                      const base::JsonType& params)
{
    if (!m_authorized)  throw ex::NotConnectedException{};
    if (method.empty()) throw ex::EmptyArgumentException{};

    const std::string target = std::string(base::VKBOT_API_METHOD_PFX) + method;
    const auto        full   = fill_required_params(params);
    const std::string raw    = m_http.post(
        std::string(base::VKBOT_API_HOST), target, params_to_query(full));
    return base::JsonType::parse(raw);
}


base::JsonType UserBase::fill_required_params(const base::JsonType& params) const {
    base::JsonType out = params;
    if (!out.contains("access_token")) out["access_token"] = m_access_token;
    if (!out.contains("v"))            out["v"]            = std::string(base::VKBOT_API_VERSION);
    return out;
}


UserBase::ValidationTypes UserBase::parse_validation_type(std::string_view desc) noexcept
{
    if (desc == "2fa_sms") {
        return ValidationTypes::TwoFaSms;
    };
    if (desc == "2fa_app") {
        return ValidationTypes::TwoFaApp;
    };
    return ValidationTypes::Unknown;
}


std::string UserBase::method_to_string(Method method)
{
    switch (method) {
    case Method::AccountBan:                           return "account.Ban";

    case Method::AccountChangePassword:                return "account.changePassword";

    case Method::AccountGetActiveOffers:               return "account.getActiveOffers";

    case Method::AccountGetAppPermissions:             return "account.getAppPermissions";

    case Method::AccountGetBanned:                     return "account.getBanned";

    case Method::AccountGetCounters:                   return "account.getCounters";

    case Method::AccountGetInfo:                       return "account.getInfo";

    case Method::AccountGetProfileInfo:                return "account.getProfileInfo";

    case Method::AccountGetPushSettings:               return "account.getPushSettings";

    case Method::AccountRegisterDevice:                return "account.registerDevice";

    case Method::AccountSaveProfileInfo:               return "account.saveProfileInfo";

    case Method::AccountSetInfo:                       return "account.setInfo";

    case Method::AccountSetPushSettings:               return "account.setPushSettings";

    case Method::AccountUnban:                         return "account.unban";

    case Method::AccountUnregisterDevice:              return "account.unregisterDevice";

    case Method::BoardAddTopic:                        return "board.addTopic";

    case Method::BoardCloseTopic:                      return "board.closeTopic";

    case Method::BoardCreateComment:                   return "board.createComment";

    case Method::BoardDeleteComment:                   return "board.deleteComment";

    case Method::BoardDeleteTopic:                     return "board.deleteTopic";

    case Method::BoardEditComment:                     return "board.editComment";
    case Method::BoardEditTopic:                       return "board.editTopic";

    case Method::BoardFixTopic:                        return "board.fixTopic";

    case Method::BoardGetComments:                     return "board.getComments";

    case Method::BoardGetTopics:                       return "board.getTopics";

    case Method::BoardOpenTopic:                       return "board.openTopic";

    case Method::BoardRestoreComment:                  return "board.restoreComment";

    case Method::BoardUnfixTopic:                      return "board.unfixTopic";

    case Method::DatabaseGetChairs:                    return "database.getChairs";

    case Method::DatabaseGetCities:                    return "database.getCities";

    case Method::DatabaseGetCitiesById:                return "database.getCitiesById";

    case Method::DatabaseGetCountries:                 return "database.getCountries";

    case Method::DatabaseGetFaculties:                 return "database.getFaculties";
    
    case Method::DatabaseGetMetroStations:             return "database.getMetroStations";

    case Method::DatabaseGetRegions:                   return "database.getRegions";

    case Method::DatabaseGetSchoolClasses:             return "database.getSchoolClasses";

    case Method::DatabaseGetSchools:                   return "database.getSchools";

    case Method::DatabaseGetUniversities:              return "database.getUniversities";

    case Method::DocsAdd:                              return "docs.add";

    case Method::DocsDelete:                           return "docs.delete";

    case Method::DocsEdit:                             return "docs.edit";

    case Method::DocsGet:                              return "docs.get";

    case Method::DocsGetById:                          return "docs.getById";

    case Method::DocsGetMessagesUploadServer:          return "docs.getMessagesUploadServer";

    case Method::DocsGetTypes:                         return "docs.getTypes";

    case Method::DocsGetUploadServer:                  return "docs.getUploadServer";

    case Method::DocsGetWallUploadServer:              return "docs.getWallUploadServer";

    case Method::DocsSave:                             return "docs.save";

    case Method::DocsSearch:                           return "docs.search";

    case Method::Execute:                              return "execute";

    case Method::FaveAddArticle:                       return "fave.addArticle";

    case Method::FaveAddLink:                          return "fave.addLink";

    case Method::FaveAddPage:                          return "fave.addPage";

    case Method::FaveAddPost:                          return "fave.addPost";

    case Method::FaveAddProduct:                       return "fave.addProduct";

    case Method::FaveAddTag:                           return "fave.addTag";

    case Method::FaveAddVideo:                         return "fave.addVideo";

    case Method::FaveEditTag:                          return "fave.editTag";

    case Method::FaveGet:                              return "fave.get";

    case Method::FaveGetPages:                         return "fave.getPages";

    case Method::FaveGetTags:                          return "fave.getTags";

    case Method::FaveMarkSeen:                         return "fave.markSeen";

    case Method::FaveRemoveArticle:                    return "fave.removeArticle";

    case Method::FaveRemoveLink:                       return "fave.removeLink";

    case Method::FaveRemovePage:                       return "fave.removePage";

    case Method::FaveRemovePost:                       return "fave.removePost";

    case Method::FaveRemoveProduct:                    return "fave.removeProduct";

    case Method::FaveRemoveTag:                        return "fave.removeTag";

    case Method::FaveRemoveVideo:                      return "fave.removeVideo";

    case Method::FaveReorderTags:                      return "fave.reorderTags";

    case Method::FaveSetPageTags:                      return "fave.setPageTags";

    case Method::FaveSetTags:                          return "fave.setTags";

    case Method::FaveTrackPageInteraction:             return "fave.trackPageInteraction";

    case Method::FriendsAdd:                           return "friends.add";

    case Method::FriendsAddList:                       return "friends.addList";

    case Method::FriendsAreFriends:                    return "friends.areFriends";

    case Method::FriendsDelete:                        return "friends.delete";

    case Method::FriendsDeleteAllRequests:             return "friends.deleteAllRequests";

    case Method::FriendsDeleteList:                    return "friends.deleteList";

    case Method::FriendsEdit:                          return "friends.edit";

    case Method::FriendsEditList:                      return "friends.editList";

    case Method::FriendsGet:                           return "friends.get";

    case Method::FriendsGetAppUsers:                   return "friends.getAppUsers";

    case Method::FriendsGetByPhones:                   return "friends.getByPhones";

    case Method::FriendsGetLists:                      return "friends.getLists";

    case Method::FriendsGetMutual:                     return "friends.getMutual";

    case Method::FriendsGetOnline:                     return "friends.getOnline";

    case Method::FriendsGetRecent:                     return "friends.getRecent";

    case Method::FriendsGetRequests:                   return "friends.getRequests";

    case Method::FriendsGetSuggestions:                return "friends.getSuggestions";

    case Method::FriendsSearch:                        return "friends.search";

    case Method::GiftsGet:                             return "gifts.get";

    case Method::GroupsAddAddress:                     return "groups.addAddress";

    case Method::GroupsAddCallbackServer:              return "groups.addCallbackServer";

    case Method::GroupsAddLink:                        return "groups.addLink";

    case Method::GroupsApproveRequest:                 return "groups.approveRequest";

    case Method::GroupsBan:                            return "groups.ban";

    case Method::GroupsCreate:                         return "groups.create";

    case Method::GroupsDeleteAddress:                  return "groups.deleteAddress";

    case Method::GroupsDeleteCallbackServer:           return "groups.deleteCallbackServer";

    case Method::GroupsDeleteLink:                     return "groups.deleteLink";

    case Method::GroupsDisableOnline:                  return "groups.disableOnline";

    case Method::GroupsEdit:                           return "groups.edit";

    case Method::GroupsEditAddress:                    return "groups.editAddress";

    case Method::GroupsEditCallbackServer:             return "groups.editCallbackServer";

    case Method::GroupsEditLink:                       return "groups.editLink";

    case Method::GroupsEditManager:                    return "groups.editManager";

    case Method::GroupsEnableOnline:                   return "groups.enableOnline";

    case Method::GroupsGet:                            return "groups.get";

    case Method::GroupsGetAddresses:                   return "groups.getAddresses";

    case Method::GroupsGetBanned:                      return "groups.getBanned";

    case Method::GroupsGetById:                        return "groups.getById";

    case Method::GroupsGetCallbackConfirmationCode:    return "groups.getCallbackConfirmationCode";

    case Method::GroupsGetCallbackServer:              return "groups.getCallbackServer";

    case Method::GroupsGetCallbackSettings:            return "groups.getCallbackSettings";

    case Method::GroupsGetCatalog:                     return "groups.getCatalog";

    case Method::GroupsGetCatalogInfo:                 return "groups.getCatalogInfo";

    case Method::GroupsGetInvitedUsers:                return "groups.getInvitedUsers";

    case Method::GroupsGetInvities:                    return "groups.getInvities";

    case Method::GroupsGetLongPollServer:              return "groups.getLongPollServer";

    case Method::GroupsGetLongPollSettings:            return "groups.getLongPollSettings";

    case Method::GroupsGetMembers:                     return "groups.getMembers";

    case Method::GroupsGetOnlineStatus:                return "groups.getOnlineStatus";

    case Method::GroupsGetRequests:                    return "groups.getRequests";

    case Method::GroupsGetSettings:                    return "groups.getSettings";

    case Method::GroupsGetTagList:                     return "groups.getTagList";

    case Method::GroupsGetTokenPermissions:            return "groups.getTokenPermissions";

    case Method::GroupsInvite:                         return "groups.invite";

    case Method::GroupsIsMember:                       return "groups.isMember";

    case Method::GroupsJoin:                           return "groups.join";

    case Method::GroupsLeave:                          return "groups.leave";

    case Method::GroupsRemoveUser:                     return "groups.removeUser";

    case Method::GroupsReorderLink:                    return "groups.reorderLink";

    case Method::GroupsSearch:                         return "groups.search";

    case Method::GroupsSetCallbackSettings:            return "groups.setCallbackSettings";

    case Method::GroupsSetLongPollSettings:            return "groups.setLongPollSettings";

    case Method::GroupsSetSettings:                    return "groups.setSettings";

    case Method::GroupsSetUserNote:                    return "groups.setUserNote";

    case Method::GroupsTagAdd:                         return "groups.tagAdd";

    case Method::GroupsTagBind:                        return "groups.tagBind";

    case Method::GroupsTagDelete:                      return "groups.tagDelete";

    case Method::GroupsTagUpdate:                      return "groups.tagUpdate";

    case Method::GroupsUnban:                          return "groups.unban";

    case Method::LeadformsCreate:                      return "leadForms.create";

    case Method::LeadformsDelete:                      return "leadForms.delete";

    case Method::LeadformsGet:                         return "leadForms.get";

    case Method::LeadformsGetLeads:                    return "leadForms.getLeads";

    case Method::LeadformsGetUploadUrl:                return "leadForms.getUploadURL";

    case Method::LeadformsList:                        return "leadForms.list";

    case Method::LeadformsUpdate:                      return "leadForms.update";

    case Method::LikesAdd:                             return "likes.add";

    case Method::LikesDelete:                          return "likes.delete";

    case Method::LikesGetList:                         return "likes.getList";

    case Method::LikesIsLiked:                         return "likes.isLiked";

    case Method::MarketAdd:                            return "market.add";

    case Method::MarketAddAlbum:                       return "market.addAlbum";

    case Method::MarketAddToAlbum:                     return "market.addToAlbum";

    case Method::MarketCreateComment:                  return "market.createComment";

    case Method::MarketDelete:                         return "market.delete";

    case Method::MarketDeleteAlbum:                    return "market.deleteAlbum";

    case Method::MarketDeleteComment:                  return "market.deleteComment";

    case Method::MarketEdit:                           return "market.edit";

    case Method::MarketEditAlbum:                      return "market.editAlbum";

    case Method::MarketEditComment:                    return "market.editComment";

    case Method::MarketEditOrder:                      return "market.editOrder";

    case Method::MarketGet:                            return "market.get";

    case Method::MarketGetAlbumById:                   return "market.getAlbumById";

    case Method::MarketGetAlbums:                      return "market.getAlbums";

    case Method::MarketGetById:                        return "market.getById";

    case Method::MarketGetCategories:                  return "market.getCategories";

    case Method::MarketGetComments:                    return "market.getComments";

    case Method::MarketGetGroupOrders:                 return "market.getGroupOrders";

    case Method::MarketGetOrderById:                   return "market.getOrderById";

    case Method::MarketGetOrderItems:                  return "market.getOrderItems";

    case Method::MarketGetOrders:                      return "market.getOrders";

    case Method::MarketRemoveFromAlbum:                return "market.removeFromAlbum";

    case Method::MarketReorderAlbums:                  return "market.reorderAlbums";

    case Method::MarketReorderItems:                   return "market.reorderItems";

    case Method::MarketReport:                         return "market.report";

    case Method::MarketReportComment:                  return "market.reportComment";

    case Method::MarketRestore:                        return "market.restore";

    case Method::MarketRestoreComment:                 return "market.restoreComment";

    case Method::MarketSearch:                         return "market.search";
    
    case Method::MessagesAddChatUser:                  return "messages.addChatUser";

    case Method::MessagesAllowMessagesFromGroup:       return "messages.allowMessagesFromGroup";

    case Method::MessagesCreateChat:                   return "messages.createChat";

    case Method::MessagesDelete:                       return "messages.delete";

    case Method::MessagesDeleteChatPhoto:              return "messages.deleteChatPhoto";

    case Method::MessagesDeleteConversation:           return "messages.deleteConversation";

    case Method::MessagesDenyMessagesFromGroup:        return "messages.denyMessagesFromGroup";

    case Method::MessagesEdit:                         return "messages.edit";

    case Method::MessagesEditChat:                     return "messages.editChat";

    case Method::MessagesGetByConversationMessageId:   return "messages.getByConversationMessageId";

    case Method::MessagesGetById:                      return "messages.getById";

    case Method::MessagesGetChat:                      return "messages.getChat";

    case Method::MessagesGetChatPreview:               return "messages.getChatPreview";

    case Method::MessagesGetConversations:             return "messages.getConversations";

    case Method::MessagesGetConversationById:          return "messages.getConversationById";

    case Method::MessagesGetHistory:                   return "messages.getHistory";

    case Method::MessagesGetHistoryAttachments:        return "messages.getHistoryAttachments";

    case Method::MessagesGetImportantMessages:         return "messages.getImportantMessages";

    case Method::MessagesGetInviteLink:                return "messages.getInviteLink";

    case Method::MessagesGetLastActivity:              return "messages.getLastActivity";

    case Method::MessagesGetLongPollHistory:           return "messages.getLongPollHistory";

    case Method::MessagesGetLongPollServer:            return "messages.getLongPollServer";

    case Method::MessagesIsMessagesFromGroupAllowed:   return "messages.isMessagesFromGroupAllowed";

    case Method::MessagesJoinChatByInviteLink:         return "messages.joinChatByInviteLink";

    case Method::MessagesMarkAsAnsweredConversation:   return "messages.markAsAnsweredConversation";

    case Method::MessagesMarkAsImportant:              return "messages.markAsImportant";

    case Method::MessagesMarkAsImportantConversation:  return "messages.markAsImportantConversation";

    case Method::MessagesMarkAsRead:                   return "messages.markAsRead";

    case Method::MessagesPin:                          return "messages.pin";

    case Method::MessagesRemoveChatUser:               return "messages.removeChatUser";

    case Method::MessagesRestore:                      return "messages.restore";

    case Method::MessagesSearch:                       return "messages.search";

    case Method::MessagesSearchConversations:          return "messages.searchConversations";

    case Method::MessagesSend:                         return "messages.send";

    case Method::MessagesSendMessageEventAnswer:       return "messages.sendMessageEventAnswer";

    case Method::MessagesSetActivity:                  return "messages.setActivity";

    case Method::MessagesSetChatPhoto:                 return "messages.setChatPhoto";

    case Method::MessagesUnpin:                        return "messages.unpin";

    case Method::NewsfeedAddBan:                       return "newsfeed.addBan";

    case Method::NewsfeedDeleteBan:                    return "newsfeed.deleteBan";

    case Method::NewsfeedDeleteList:                   return "newsfeed.deleteList";

    case Method::NewsfeedGet:                          return "newsfeed.get";

    case Method::NewsfeedGetBanned:                    return "newsfeed.getBanned";

    case Method::NewsfeedGetComments:                  return "newsfeed.getComments";

    case Method::NewsfeedGetLists:                     return "newsfeed.getLists";

    case Method::NewsfeedGetMentions:                  return "newsfeed.getMentions";

    case Method::NewsfeedGetRecommended:               return "newsfeed.getRecommended";

    case Method::NewsfeedGetSuggestedSources:          return "newsfeed.getSuggestedSources";

    case Method::NewsfeedIgnoreItem:                   return "newsfeed.ignoreItem";

    case Method::NewsfeedSaveList:                     return "newsfeed.saveList";

    case Method::NewsfeedSearch:                       return "newsfeed.search";

    case Method::NewsfeedUnignoredItem:                return "newsfeed.unignoredItem";

    case Method::NewsfeedUnsubscribe:                  return "newsfeed.unsubscribe";

    case Method::NotesAdd:                             return "notes.add";

    case Method::NotesCreateComment:                   return "notes.createComment";

    case Method::NotesDelete:                          return "notes.delete";

    case Method::NotesDeleteComment:                   return "notes.deleteComment";

    case Method::NotesEdit:                            return "notes.edit";

    case Method::NotesEditComment:                     return "notes.editComment";

    case Method::NotesGet:                             return "notes.get";

    case Method::NotesGetById:                         return "notes.getById";

    case Method::NotesGetComments:                     return "notes.getComments";

    case Method::NotesRestoreComments:                 return "notes.restoreComments";

    case Method::NotificationsGet:                     return "notifications.get";

    case Method::NotificationsMarkAsViewed:            return "notifications.markAsViewed";

    case Method::PagesGet:                             return "pages.get";

    case Method::PagesGetHistory:                      return "pages.getHistory";

    case Method::PagesGetTitles:                       return "pages.getTitles";

    case Method::PagesGetVersion:                      return "pages.getVersion";

    case Method::PagesParseWiki:                       return "pages.parseWiki";

    case Method::PagesSave:                            return "pages.save";

    case Method::PagesSaveAccess:                      return "pages.saveAccess";

    case Method::PhotosConfirmTag:                     return "photos.confirmTag";

    case Method::PhotosCopy:                           return "photos.copy";

    case Method::PhotosCreateAlbum:                    return "photos.createAlbum";

    case Method::PhotosCreateComment:                  return "photos.createComment";

    case Method::PhotosDelete:                         return "photos.delete";

    case Method::PhotosDeleteAlbum:                    return "photos.deleteAlbum";

    case Method::PhotosDeleteComment:                  return "photos.deleteComment";

    case Method::PhotosEdit:                           return "photos.edit";

    case Method::PhotosEditAlbum:                      return "photos.editAlbum";

    case Method::PhotosEditComment:                    return "photos.editComment";

    case Method::PhotosGet:                            return "photos.get";

    case Method::PhotosGetAlbum:                       return "photos.getAlbum";

    case Method::PhotosGetAlbumsCount:                 return "photos.getAlbumsCount";

    case Method::PhotosGetAll:                         return "photos.getAll";

    case Method::PhotosGetAllComments:                 return "photos.getAllComments";

    case Method::PhotosGetById:                        return "photos.getById";

    case Method::PhotosGetChatUploadServer:            return "photos.getChatUploadServer";

    case Method::PhotosGetComments:                    return "photos.getComments";

    case Method::PhotosGetMarketAlbumUploadServer:     return "photos.getMarketAlbumUploadServer";

    case Method::PhotosGetMarketUploadServer:          return "photos.getMarketUploadServer";

    case Method::PhotosGetMessagesUploadServer:        return "photos.getMessagesUploadServer";

    case Method::PhotosGetNewTags:                     return "photos.getNewTags";

    case Method::PhotosGetOwnerCoverPhotoUploadServer: return "photos.getOwnerCoverPhotoUploadServer";
    case Method::PhotosGetOwnerPhotoUploadServer:      return "photos.getOwnerPhotoUploadServer";

    case Method::PhotosGetTags:                        return "photos.getTags";

    case Method::PhotosGetUploadServer:                return "photos.getUploadServer";

    case Method::PhotosGetUserPhotos:                  return "photos.getUserPhotos";

    case Method::PhotosGetWallUploadServer:            return "photos.getWallUploadServer";

    case Method::PhotosMakeCover:                      return "photos.makeCover";

    case Method::PhotosMove:                           return "photos.move";

    case Method::PhotosPutTag:                         return "photos.putTag";

    case Method::PhotosRemoveTag:                      return "photos.removeTag";

    case Method::PhotosReorderAlbums:                  return "photos.reorderAlbums";

    case Method::PhotosReorderPhotos:                  return "photos.reorderPhotos";

    case Method::PhotosReport:                         return "photos.report";

    case Method::PhotosReportComment:                  return "photos.reportComment";

    case Method::PhotosRestore:                        return "photos.restore";

    case Method::PhotosRestoreComment:                 return "photos.restoreComment";

    case Method::PhotosSave:                           return "photos.save";

    case Method::PhotosSaveMarketAlbumPhoto:           return "photos.saveMarketAlbumPhoto";

    case Method::PhotosSaveMarketPhoto:                return "photos.saveMarketPhoto";

    case Method::PhotosSaveMessagesPhoto:              return "photos.saveMessagesPhoto";

    case Method::PhotosSaveOwnerCoverPhoto:            return "photos.saveOwnerCoverPhoto";

    case Method::PhotosSaveOwnerPhoto:                 return "photos.saveOwnerPhoto";

    case Method::PhotosSaveWallPhoto:                  return "photos.saveWallPhoto";

    case Method::PhotosSearch:                         return "photos.search";

    case Method::PollsAddVote:                         return "polls.addVote";

    case Method::PollsCreate:                          return "polls.create";

    case Method::PollsDeleteVote:                      return "polls.deleteVote";

    case Method::PollsEdit:                            return "polls.edit";

    case Method::PollsGetBackgrounds:                  return "polls.getBackgrounds";

    case Method::PollsGetById:                         return "polls.getById";

    case Method::PollsGetPhotoUploadServer:            return "polls.getPhotoUploadServer";

    case Method::PollsGetVotes:                        return "polls.getVotes";

    case Method::PollsSavePhoto:                       return "polls.savePhoto";

    case Method::PrettycardsCreate:                    return "prettycards.create";

    case Method::PrettycardsDelete:                    return "prettycards.delete";

    case Method::PrettycardsEdit:                      return "prettycards.edit";

    case Method::PrettycardsGet:                       return "prettycards.get";

    case Method::PrettycardsGetById:                   return "prettycards.getById";

    case Method::PrettycardsGetUploadUrl:              return "prettycards.getUploadURL";

    case Method::SearchGetHints:                       return "search.getHints";

    case Method::StatsGet:                             return "stats.get";

    case Method::StatsGetPostReach:                    return "stats.getPostReach";

    case Method::StatsTrackVisitor:                    return "stats.trackVisitor";

    case Method::StatusGet:                            return "status.get";

    case Method::StatusSet:                            return "status.set";

    case Method::StoriesBanOwner:                      return "stories.banOwner";

    case Method::StoriesDelete:                        return "stories.delete";

    case Method::StoriesGet:                           return "stories.get";

    case Method::StoriesGetBanned:                     return "stories.getBanned";

    case Method::StoriesGetById:                       return "stories.getById";

    case Method::StoriesGetPhotoUploadServer:          return "stories.getPhotoUploadServer";

    case Method::StoriesGetReplies:                    return "stories.getReplies";

    case Method::StoriesGetStats:                      return "stories.getStats";

    case Method::StoriesGetVideoUploadServer:          return "stories.getVideoUploadServer";

    case Method::StoriesGetViewers:                    return "stories.getViewers";

    case Method::StoriesHideAllReplies:                return "stories.hideAllReplies";

    case Method::StoriesHideReply:                     return "stories.hideReply";

    case Method::StoriesSave:                          return "stories.save";

    case Method::StoriesSearch:                        return "stories.search";

    case Method::StoriesUnbanOwner:                    return "stories.unbanOwner";

    case Method::UsersGet:                             return "users.get";

    case Method::UsersGetFollowers:                    return "users.getFollowers";

    case Method::UsersGetSubscriptions:                return "users.getSubscriptions";

    case Method::UsersReport:                          return "users.report";

    case Method::UsersSearch:                          return "users.search";

    case Method::UtilsCheckLink:                       return "utils.checkLink";

    case Method::VideoAdd:                             return "video.add";

    case Method::VideoAddAlbum:                        return "video.addAlbum";

    case Method::VideoAddToAlbum:                      return "video.addToAlbum";

    case Method::VideoCreateComment:                   return "video.createComment";

    case Method::VideoDelete:                          return "video.delete";

    case Method::VideoDeleteAlbum:                     return "video.deleteAlbum";

    case Method::VideoDeleteComment:                   return "video.deleteComment";

    case Method::VideoEdit:                            return "video.edit";

    case Method::VideoEditAlbum:                       return "video.editAlbum";

    case Method::VideoEditComment:                     return "video.editComment";

    case Method::VideoGet:                             return "video.get";

    case Method::VideoGetAlbumById:                    return "video.getAlbumById";

    case Method::VideoGetAlbums:                       return "video.getAlbums";

    case Method::VideoGetAlbumsByVideo:                return "video.getAlbumsByVideo";

    case Method::VideoGetComments:                     return "video.getComments";

    case Method::VideoRemoveFromAlbums:                return "video.removeFromAlbums";

    case Method::VideoReorderAlbums:                   return "video.reorderAlbums";

    case Method::VideoReorderVideos:                   return "video.reorderVideos";

    case Method::VideoReport:                          return "video.report";

    case Method::VideoReportComment:                   return "video.reportComment";

    case Method::VideoRestore:                         return "video.restore";

    case Method::VideoRestoreComment:                  return "video.restoreComment";

    case Method::VideoSave:                            return "video.save";

    case Method::VideoSearch:                          return "video.search";

    case Method::WallCheckCopyrightLink:               return "wall.checkCopyrightLink"
    ;
    case Method::WallCloseComments:                    return "wall.closeComments";

    case Method::WallCreateComment:                    return "wall.createComment";

    case Method::WallDelete:                           return "wall.delete";

    case Method::WallDeleteComment:                    return "wall.deleteComment";

    case Method::WallEdit:                             return "wall.edit";

    case Method::WallEditAdsStealth:                   return "wall.editAdsStealth";

    case Method::WallEditComment:                      return "wall.editComment";

    case Method::WallGet:                              return "wall.get";

    case Method::WallGetById:                          return "wall.getById";

    case Method::WallGetComment:                       return "wall.getComment";

    case Method::WallGetReposts:                       return "wall.getReposts";

    case Method::WallOpenComments:                     return "wall.openComments";

    case Method::WallPin:                              return "wall.pin";

    case Method::WallPost:                             return "wall.post";

    case Method::WallPostAdsStealth:                   return "wall.postAdsStealth";

    case Method::WallReportComment:                    return "wall.reportComment";

    case Method::WallReportPost:                       return "wall.reportPost";

    case Method::WallRepost:                           return "wall.repost";

    case Method::WallRestore:                          return "wall.restore";

    case Method::WallRestoreComment:                   return "wall.restoreComment";

    case Method::WallSearch:                           return "wall.search";

    case Method::WallUnpin:                            return "wall.unpin";

    }
    return {};
}

} // namespace vk::user
