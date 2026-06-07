/**
 * 使用流程：
 *   1. stateCreat()     — 註冊所有狀態
 *   2. stateSetTrans()  — 註冊狀態之間的轉移規則
 *   3. start()          — 指定初始狀態，啟動狀態機
 *   4. stateUpdate()    — 在主迴圈中反覆呼叫，驅動狀態轉移
 */

#pragma once
#include <cstdint>
#include <cstring>

#define STATECOUNT 10                                   ///< 最大狀態數量
#define STATE_CONNECTCOUNT STATECOUNT *(STATECOUNT - 1) ///< 最大轉移規則數量（完全圖）

class FSM
{
private:
    /// @brief 狀態描述
    typedef struct
    {
        uint8_t     id;         ///< 狀態 ID（即 stateForm 陣列索引）
        const char *name;       ///< 狀態名稱（僅用於調試顯示，不參與邏輯比對）
        void      (*handle)();  ///< 狀態事件回調：處於此狀態時每幀執行（可為 nullptr）
    } stateData_t;

    /// @brief 轉移規則
    typedef struct
    {
        uint8_t fromID;         ///< 來源狀態 ID
        uint8_t toID;           ///< 目標狀態 ID
        bool   (*reason)();     ///< 守衛函數：返回 true 時觸發轉移
    } stateTrans_t;

    // ──────────────────────────────────────────────
    // 內部資料（全靜態分配）
    // ──────────────────────────────────────────────

    stateTrans_t stateTransForm[STATE_CONNECTCOUNT];    ///< 轉移規則表
    stateData_t  stateForm[STATECOUNT] = {0};           ///< 狀態表（{0} 將所有 name/handle 初始為 nullptr）
    uint8_t      currentID = 0xff;                      ///< 當前狀態 ID（0xff = 未啟動）
    uint16_t     transCount = 0;                        ///< 已註冊的轉移規則數量

public:
    // ──────────────────────────────────────────────
    // 狀態管理
    // ──────────────────────────────────────────────

    /**
     * @brief   註冊一個新狀態
     *
     * @param   name  狀態名稱（建議傳字串字面量，如 "IDLE"）
     * @param   func  狀態事件回調，在該狀態期間每幀執行（可為 nullptr）
     * @return  true  註冊成功
     * @return  false 狀態表已滿（STATECOUNT 個）
     *
     * @note    狀態 ID 由內部自動分配，等於其在 stateForm 中的索引
     */
    bool stateCreat(const char *name, void (*func)())
    {
        for (int i = 0; i < STATECOUNT; i++)
        {
            if (stateForm[i].name == nullptr)       // 找到空位
            {
                stateForm[i].id     = i;
                stateForm[i].name   = name;
                stateForm[i].handle = func;
                return true;
            }
        }
        return false;                               // 狀態表已滿
    }

    /**
     * @brief   刪除一個已註冊的狀態
     *
     * @param   name  要刪除的狀態名稱（需與註冊時完全相同）
     * @return  true  刪除成功
     * @return  false 未找到該名稱的狀態
     *
     * @note    刪除後僅清空 name 與 handle，該槽位可被後續 stateCreat 復用
     * @warning 不會自動清理引用此狀態的轉移規則，需用戶自行處理
     */
    bool stateDelete(const char *name)
    {
        for (int i = 0; i < STATECOUNT; i++)
        {
            if (stateForm[i].name && !strcmp(stateForm[i].name, name))
            {
                stateForm[i].name   = nullptr;
                stateForm[i].handle = nullptr;
                return true;
            }
        }
        return false;
    }

    // ──────────────────────────────────────────────
    // 轉移規則管理
    // ──────────────────────────────────────────────

    /**
     * @brief   註冊一條狀態轉移規則
     *
     * @param   from  來源狀態名稱
     * @param   to    目標狀態名稱
     * @param   func  守衛函數（guard）：當 FSM 處於 from 狀態且此函數返回 true 時觸發轉移
     *                傳 nullptr 表示無條件轉移（每幀皆觸發）
     * @return  true  註冊成功
     * @return  false 來源或目標狀態不存在，或轉移表已滿
     *
     * @note    每條轉移規則為單向：from → to
     *          若需雙向轉移，請呼叫兩次（A→B 與 B→A）
     */
    bool stateSetTrans(const char *from, const char *to, bool (*func)())
    {
        uint8_t fromId = 0xff, toId = 0xff;

        // 依名稱查找來源與目標狀態的 ID
        for (int i = 0; i < STATECOUNT; i++)
        {
            if (stateForm[i].name && !strcmp(stateForm[i].name, from))
                fromId = i;
            if (stateForm[i].name && !strcmp(stateForm[i].name, to))
                toId   = i;
        }

        if (fromId == 0xFF || toId == 0xFF)
            return false;                           // 狀態不存在

        stateTransForm[transCount] = {fromId, toId, func};
        transCount++;
        return true;
    }


    /**
     * @brief   啟動狀態機，設定初始狀態
     *
     * @param   startName  初始狀態名稱
     *
     * @note    必須在 stateUpdate() 之前呼叫
     *          若名稱不存在，currentID 會被設為 0xff（未啟動），
     *          後續 stateUpdate() 將直接返回不執行任何操作
     */
    void start(const char *startName)
    {
        for (int i = 0; i < STATECOUNT; i++)
        {
            if (stateForm[i].name && !strcmp(stateForm[i].name, startName))
            {
                currentID = i;                      // 找到，設定初始狀態
                return;
            }
        }
        currentID = 0xff;                           // 未找到，保持未啟動
    }

    /**
     * @brief   驅動狀態機（每幀呼叫一次）
     *
     * 執行順序：
     *   1. 若未啟動（currentID == 0xff），直接返回
     *   2. 遍歷轉移表，檢查是否有從當前狀態出發且守衛函數返回 true 的規則
     *   3. 若找到，切換到目標狀態（一次最多跳一條規則）
     *   4. 執行當前狀態的事件回調（handle）
     *
     * @note    應放在主迴圈或 RTOS 任務中反覆呼叫
     *          守衛函數應輕量且非阻塞（僅檢查旗標/信號量/IO）
     */
    void stateUpdate()
    {
        if (currentID == 0xff)
            return;                                 // 未啟動，不執行

        // 掃描轉移表，檢查是否有符合條件的轉移
        for (int i = 0; i < transCount; i++)
        {
            if (stateTransForm[i].fromID == currentID
                && stateTransForm[i].reason
                && stateTransForm[i].reason())
            {
                currentID = stateTransForm[i].toID; // 執行轉移
                break;                              // 一幀最多觸發一次轉移
            }
        }

        // 執行當前狀態的事件回調
        if (stateForm[currentID].handle)
            stateForm[currentID].handle();
    }
};
