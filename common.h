#ifndef _COMMON_H_
#define _COMMON_H_


enum AncsNotificationEventId {
  AncsNotificationEventIdAdded    = 0,
  AncsNotificationEventIdModified = 1,
  AncsNotificationEventIdRemoved  = 2
};

enum AncsNotificationEventFlags {
  AncsNotificationEventFlagsSilent         = 1,
  AncsNotificationEventFlagsImportant      = 2,
  AncsNotificationEventFlagsPositiveAction = 4,
  AncsNotificationEventFlagsNegativeAction = 8
};

enum AncsNotificationCategoryId {
  AncsNotificationCategoryIdOther              = 0,
  AncsNotificationCategoryIdIncomingCall       = 1,
  AncsNotificationCategoryIdMissedCall         = 2,
  AncsNotificationCategoryIdVoicemail          = 3,
  AncsNotificationCategoryIdSocial             = 4,
  AncsNotificationCategoryIdSchedule           = 5,
  AncsNotificationCategoryIdEmail              = 6,
  AncsNotificationCategoryIdNews               = 7,
  AncsNotificationCategoryIdHealthAndFitness   = 8,
  AncsNotificationCategoryIdBusinessAndFinance = 9,
  AncsNotificationCategoryIdLocation           = 10,
  AncsNotificationCategoryIdEntertainment      = 11
};

struct AncsNotification {
  unsigned char eventId;
  unsigned char eventFlags;
  unsigned char catergoryId;
  unsigned char catergoryCount;
  unsigned long notificationUid;
};



#define ANCS_COMMAND_GET_NOTIF_ATTRIBUTES 0x0

#define ANCS_NOTIFICATION_ATTRIBUTE_APP_IDENTIFIER 0x0
#define ANCS_NOTIFICATION_ATTRIBUTE_TITLE          0x1 // shall be followed by 2bytes max length
#define ANCS_NOTIFICATION_ATTRIBUTE_SUBTITLE       0x2 // shall be followed by 2bytes max length
#define ANCS_NOTIFICATION_ATTRIBUTE_MESSAGE        0x3 // shall be followed by 2bytes max length
#define ANCS_NOTIFICATION_ATTRIBUTE_MESSAGE_SIZE   0x4
#define ANCS_NOTIFICATION_ATTRIBUTE_DATE           0x5

#define TITLE_LENGTH 32
#define MESSAGE_LENGTH 140


#endif
