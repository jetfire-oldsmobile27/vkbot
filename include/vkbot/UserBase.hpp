/**
 * @file UserBase.hpp
 * @brief Клиент для работы с VK User Long Poll API.
 *
 * @version 0.1.0
 */

#pragma once

#include <optional>
#include <string>

#include <vkbot/ClientBase.hpp>

namespace vk::user {

class UserBase final : public base::ClientBase {
public:
    
    enum class Method { /*@todo */
        AccountBan, AccountChangePassword, AccountGetActiveOffers,
        AccountGetAppPermissions, AccountGetBanned, AccountGetCounters,
        AccountGetInfo, AccountGetProfileInfo, AccountGetPushSettings,
        AccountRegisterDevice, AccountSaveProfileInfo, AccountSetInfo,
        AccountSetPushSettings, AccountUnban, AccountUnregisterDevice,
        BoardAddTopic, BoardCloseTopic, BoardCreateComment, BoardDeleteComment,
        BoardDeleteTopic, BoardEditComment, BoardEditTopic, BoardFixTopic,
        BoardGetComments, BoardGetTopics, BoardOpenTopic, BoardRestoreComment,
        BoardUnfixTopic,
        DatabaseGetChairs, DatabaseGetCities, DatabaseGetCitiesById,
        DatabaseGetCountries, DatabaseGetFaculties, DatabaseGetMetroStations,
        DatabaseGetRegions, DatabaseGetSchoolClasses, DatabaseGetSchools,
        DatabaseGetUniversities,
        DocsAdd, DocsDelete, DocsEdit, DocsGet, DocsGetById,
        DocsGetMessagesUploadServer, DocsGetTypes, DocsGetUploadServer,
        DocsGetWallUploadServer, DocsSave, DocsSearch,
        Execute,
        FaveAddArticle, FaveAddLink, FaveAddPage, FaveAddPost, FaveAddProduct,
        FaveAddTag, FaveAddVideo, FaveEditTag, FaveGet, FaveGetPages,
        FaveGetTags, FaveMarkSeen, FaveRemoveArticle, FaveRemoveLink,
        FaveRemovePage, FaveRemovePost, FaveRemoveProduct, FaveRemoveTag,
        FaveRemoveVideo, FaveReorderTags, FaveSetPageTags, FaveSetTags,
        FaveTrackPageInteraction,
        FriendsAdd, FriendsAddList, FriendsAreFriends, FriendsDelete,
        FriendsDeleteAllRequests, FriendsDeleteList, FriendsEdit,
        FriendsEditList, FriendsGet, FriendsGetAppUsers, FriendsGetByPhones,
        FriendsGetLists, FriendsGetMutual, FriendsGetOnline, FriendsGetRecent,
        FriendsGetRequests, FriendsGetSuggestions, FriendsSearch,
        GiftsGet,
        GroupsAddAddress, GroupsAddCallbackServer, GroupsAddLink,
        GroupsApproveRequest, GroupsBan, GroupsCreate, GroupsDeleteAddress,
        GroupsDeleteCallbackServer, GroupsDeleteLink, GroupsDisableOnline,
        GroupsEdit, GroupsEditAddress, GroupsEditCallbackServer, GroupsEditLink,
        GroupsEditManager, GroupsEnableOnline, GroupsGet, GroupsGetAddresses,
        GroupsGetBanned, GroupsGetById, GroupsGetCallbackConfirmationCode,
        GroupsGetCallbackServer, GroupsGetCallbackSettings, GroupsGetCatalog,
        GroupsGetCatalogInfo, GroupsGetInvitedUsers, GroupsGetInvities,
        GroupsGetLongPollServer, GroupsGetLongPollSettings, GroupsGetMembers,
        GroupsGetOnlineStatus, GroupsGetRequests, GroupsGetSettings,
        GroupsGetTagList, GroupsGetTokenPermissions, GroupsInvite,
        GroupsIsMember, GroupsJoin, GroupsLeave, GroupsRemoveUser,
        GroupsReorderLink, GroupsSearch, GroupsSetCallbackSettings,
        GroupsSetLongPollSettings, GroupsSetSettings, GroupsSetUserNote,
        GroupsTagAdd, GroupsTagBind, GroupsTagDelete, GroupsTagUpdate,
        GroupsUnban,
        LeadformsCreate, LeadformsDelete, LeadformsGet, LeadformsGetLeads,
        LeadformsGetUploadUrl, LeadformsList, LeadformsUpdate,
        LikesAdd, LikesDelete, LikesGetList, LikesIsLiked,
        MarketAdd, MarketAddAlbum, MarketAddToAlbum, MarketCreateComment,
        MarketDelete, MarketDeleteAlbum, MarketDeleteComment, MarketEdit,
        MarketEditAlbum, MarketEditComment, MarketEditOrder, MarketGet,
        MarketGetAlbumById, MarketGetAlbums, MarketGetById, MarketGetCategories,
        MarketGetComments, MarketGetGroupOrders, MarketGetOrderById,
        MarketGetOrderItems, MarketGetOrders, MarketRemoveFromAlbum,
        MarketReorderAlbums, MarketReorderItems, MarketReport,
        MarketReportComment, MarketRestore, MarketRestoreComment, MarketSearch,
        MessagesAddChatUser, MessagesAllowMessagesFromGroup, MessagesCreateChat,
        MessagesDelete, MessagesDeleteChatPhoto, MessagesDeleteConversation,
        MessagesDenyMessagesFromGroup, MessagesEdit, MessagesEditChat,
        MessagesGetByConversationMessageId, MessagesGetById, MessagesGetChat,
        MessagesGetChatPreview, MessagesGetConversations,
        MessagesGetConversationById, MessagesGetHistory,
        MessagesGetHistoryAttachments, MessagesGetImportantMessages,
        MessagesGetInviteLink, MessagesGetLastActivity,
        MessagesGetLongPollHistory, MessagesGetLongPollServer,
        MessagesIsMessagesFromGroupAllowed, MessagesJoinChatByInviteLink,
        MessagesMarkAsAnsweredConversation, MessagesMarkAsImportant,
        MessagesMarkAsImportantConversation, MessagesMarkAsRead, MessagesPin,
        MessagesRemoveChatUser, MessagesRestore, MessagesSearch,
        MessagesSearchConversations, MessagesSend,
        MessagesSendMessageEventAnswer, MessagesSetActivity,
        MessagesSetChatPhoto, MessagesUnpin,
        NewsfeedAddBan, NewsfeedDeleteBan, NewsfeedDeleteList, NewsfeedGet,
        NewsfeedGetBanned, NewsfeedGetComments, NewsfeedGetLists,
        NewsfeedGetMentions, NewsfeedGetRecommended,
        NewsfeedGetSuggestedSources, NewsfeedIgnoreItem, NewsfeedSaveList,
        NewsfeedSearch, NewsfeedUnignoredItem, NewsfeedUnsubscribe,
        NotesAdd, NotesCreateComment, NotesDelete, NotesDeleteComment,
        NotesEdit, NotesEditComment, NotesGet, NotesGetById, NotesGetComments,
        NotesRestoreComments,
        NotificationsGet, NotificationsMarkAsViewed,
        PagesGet, PagesGetHistory, PagesGetTitles, PagesGetVersion,
        PagesParseWiki, PagesSave, PagesSaveAccess,
        PhotosConfirmTag, PhotosCopy, PhotosCreateAlbum, PhotosCreateComment,
        PhotosDelete, PhotosDeleteAlbum, PhotosDeleteComment, PhotosEdit,
        PhotosEditAlbum, PhotosEditComment, PhotosGet, PhotosGetAlbum,
        PhotosGetAlbumsCount, PhotosGetAll, PhotosGetAllComments,
        PhotosGetById, PhotosGetChatUploadServer, PhotosGetComments,
        PhotosGetMarketAlbumUploadServer, PhotosGetMarketUploadServer,
        PhotosGetMessagesUploadServer, PhotosGetNewTags,
        PhotosGetOwnerCoverPhotoUploadServer, PhotosGetOwnerPhotoUploadServer,
        PhotosGetTags, PhotosGetUploadServer, PhotosGetUserPhotos,
        PhotosGetWallUploadServer, PhotosMakeCover, PhotosMove, PhotosPutTag,
        PhotosRemoveTag, PhotosReorderAlbums, PhotosReorderPhotos,
        PhotosReport, PhotosReportComment, PhotosRestore, PhotosRestoreComment,
        PhotosSave, PhotosSaveMarketAlbumPhoto, PhotosSaveMarketPhoto,
        PhotosSaveMessagesPhoto, PhotosSaveOwnerCoverPhoto,
        PhotosSaveOwnerPhoto, PhotosSaveWallPhoto, PhotosSearch,
        PollsAddVote, PollsCreate, PollsDeleteVote, PollsEdit,
        PollsGetBackgrounds, PollsGetById, PollsGetPhotoUploadServer,
        PollsGetVotes, PollsSavePhoto,
        PrettycardsCreate, PrettycardsDelete, PrettycardsEdit, PrettycardsGet,
        PrettycardsGetById, PrettycardsGetUploadUrl,
        SearchGetHints,
        StatsGet, StatsGetPostReach, StatsTrackVisitor,
        StatusGet, StatusSet,
        StoriesBanOwner, StoriesDelete, StoriesGet, StoriesGetBanned,
        StoriesGetById, StoriesGetPhotoUploadServer, StoriesGetReplies,
        StoriesGetStats, StoriesGetVideoUploadServer, StoriesGetViewers,
        StoriesHideAllReplies, StoriesHideReply, StoriesSave, StoriesSearch,
        StoriesUnbanOwner,
        UsersGet, UsersGetFollowers, UsersGetSubscriptions, UsersReport,
        UsersSearch,
        UtilsCheckLink,
        VideoAdd, VideoAddAlbum, VideoAddToAlbum, VideoCreateComment,
        VideoDelete, VideoDeleteAlbum, VideoDeleteComment, VideoEdit,
        VideoEditAlbum, VideoEditComment, VideoGet, VideoGetAlbumById,
        VideoGetAlbums, VideoGetAlbumsByVideo, VideoGetComments,
        VideoRemoveFromAlbums, VideoReorderAlbums, VideoReorderVideos,
        VideoReport, VideoReportComment, VideoRestore, VideoRestoreComment,
        VideoSave, VideoSearch,
        WallCheckCopyrightLink, WallCloseComments, WallCreateComment,
        WallDelete, WallDeleteComment, WallEdit, WallEditAdsStealth,
        WallEditComment, WallGet, WallGetById, WallGetComment, WallGetReposts,
        WallOpenComments, WallPin, WallPost, WallPostAdsStealth,
        WallReportComment, WallReportPost, WallRepost, WallRestore,
        WallRestoreComment, WallSearch, WallUnpin,
    };

    enum class ValidationTypes { TwoFaSms, TwoFaApp, Unknown };

    // -----------------------------------------------------------------------
    /**
     * @param app_id         ID приложения VK.
     * @param app_secure_key Защищённый ключ приложения.
     * @throws ex::EmptyArgumentException если любой из параметров пуст.
     */
    UserBase(std::string app_id, std::string app_secure_key);
    ~UserBase() override = default;

    // -----------------------------------------------------------------------
    // ClientBase interface
    // -----------------------------------------------------------------------

    /**
     * @brief Авторизация по токену доступа.
     *
     */
    bool auth(const std::string& access_token) override;

    /**
     * @brief Авторизация по логину и паролю (с поддержкой 2FA и капчи).
     */
    bool auth(const std::string& login, const std::string& password);

    base::JsonType send_request(const std::string& method,
                                const base::JsonType& params) override;

    // -----------------------------------------------------------------------
    // User-specific API
    // -----------------------------------------------------------------------
    base::JsonType send_request(Method method, const base::JsonType& params);

    [[nodiscard]] static std::string method_to_string(Method method);
    [[nodiscard]] static ValidationTypes parse_validation_type(std::string_view desc) noexcept;

protected:
    [[nodiscard]] base::JsonType fill_required_params(const base::JsonType& params) const override;

private:
    bool handle_2fa(base::JsonType& params, const base::JsonType& error_response);

    std::string m_app_id;
    std::string m_app_secure_key;
    std::string m_access_token;
    std::string m_user_id;
};

} // namespace vk::user
