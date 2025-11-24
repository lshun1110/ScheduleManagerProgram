// share_mgr.h
#ifndef SHARE_MGR_H
#define SHARE_MGR_H

#include "struct.h"

// 공유 관리 화면 표시
void ShareMgr_Show(void);

// 내가 공유한 캘린더 조회
int ShareMgr_GetSharedByMe(const wchar_t* user_id, Share* buf, int max_count);

// 나에게 공유된 캘린더 조회
int ShareMgr_GetSharedToMe(const wchar_t* user_id, Share* buf, int max_count);

// 공유받은 캘린더 정보 조회
int ShareMgr_GetSharedCalendars(const wchar_t* user_id, Calendar* buf, int max_count);

#endif
