/*
** $Id: sch_cmds.c 1.5 2017/06/21 15:29:02EDT mdeschu Exp  $
**
**  Copyright (c) 2007-2014 United States Government as represented by the 
**  Administrator of the National Aeronautics and Space Administration. 
**  All Other Rights Reserved.  
**
**  This software was created at NASA's Goddard Space Flight Center.
**  This software is governed by the NASA Open Source Agreement and may be 
**  used, distributed and modified only pursuant to the terms of that 
**  agreement.
**
** Purpose: Scheduler (SCH) application command handling
**
** Author:
**
** Notes:
**
*/

/*************************************************************************
**
** Include section
**
**************************************************************************/

#include "cfe.h"
#include "sch_msgids.h"

#include "sch_msg.h"
#include "sch_events.h"
#include "sch_app.h"
#include "sch_cmds.h"
#include "sch_version.h"

/*************************************************************************
**
** Exported data
**
**************************************************************************/

/*
** Application global data
*/
extern SCH_AppData_t           SCH_AppData;

/*************************************************************************
**
** File data
**
**************************************************************************/

/*
** (none)
*/

/*******************************************************************
**
** SCH_AppPipe
**
** NOTE: For complete prolog information, see 'sch_cmds.h'
********************************************************************/

int32 SCH_AppPipe(CFE_SB_Buffer_t *SBBufPtr)
{
    int32          Result      = CFE_SUCCESS;
    CFE_MSG_FcnCode_t         CommandCode = 0;

    CFE_MSG_Message_t *MessagePtr = &SBBufPtr->Msg;

    CFE_SB_MsgId_t MessageID = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_GetMsgId(MessagePtr, &MessageID);

    switch (MessageID.Value)
    {
    /*
    ** Housekeeping telemetry request
    */
    case SCH_SEND_HK_MID:
        Result = SCH_HousekeepingCmd(MessagePtr);
        break;

    /*
    ** SCH ground commands
    */
    case SCH_CMD_MID:

        CFE_MSG_GetFcnCode(MessagePtr, &CommandCode);
        switch (CommandCode)
        {
        case SCH_NOOP_CC:
            SCH_NoopCmd(MessagePtr);
            break;

        case SCH_RESET_CC:
            SCH_ResetCmd(MessagePtr);
            break;

        case SCH_ENABLE_CC:
            SCH_EnableCmd(MessagePtr);
            break;

        case SCH_DISABLE_CC:
            SCH_DisableCmd(MessagePtr);
            break;

        case SCH_ENABLE_GROUP_CC:
            SCH_EnableGroupCmd(MessagePtr);
            break;

        case SCH_DISABLE_GROUP_CC:
            SCH_DisableGroupCmd(MessagePtr);
            break;

        case SCH_ENABLE_SYNC_CC:
            SCH_EnableSyncCmd(MessagePtr);
            break;

        case SCH_SEND_DIAG_TLM_CC:
            SCH_SendDiagTlmCmd(MessagePtr);
            break;

        /*
        ** SCH ground commands with unknown command codes...
        */
        default:
            CFE_EVS_SendEvent(SCH_CC_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "Invalid command code: ID = 0x%04X, CC = %d",
                              MessageID.Value,
                              CommandCode);

            SCH_AppData.ErrCounter++;
            break;
        }
        break;

    /*
    ** Unknown message ID's
    */
    default:
        CFE_EVS_SendEvent(SCH_MD_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "Msg with Invalid message ID Rcvd -- ID = 0x%04X",
                          MessageID.Value);
        break;
    }

    return(Result);

} /* End of SCH_AppPipe() */


/*******************************************************************
**
** SCH_HousekeepingCmd
**
** NOTE: For complete prolog information, see above
********************************************************************/

int32 SCH_HousekeepingCmd(CFE_MSG_Message_t *MessagePtr)
{
    int32 TableResult = SCH_SUCCESS;

    if (SCH_VerifyCmdLength(MessagePtr, sizeof(SCH_NoArgsCmd_t)) == SCH_SUCCESS)
    {
        /*
        ** Update contents of Housekeeping Packet
        */
        SCH_AppData.HkPacket.CmdCounter                   = SCH_AppData.CmdCounter;
        SCH_AppData.HkPacket.ErrCounter                   = SCH_AppData.ErrCounter;
        SCH_AppData.HkPacket.ScheduleActivitySuccessCount = SCH_AppData.ScheduleActivitySuccessCount;
        SCH_AppData.HkPacket.ScheduleActivityFailureCount = SCH_AppData.ScheduleActivityFailureCount;
        SCH_AppData.HkPacket.SlotsProcessedCount          = SCH_AppData.SlotsProcessedCount;
        SCH_AppData.HkPacket.SkippedSlotsCount            = SCH_AppData.SkippedSlotsCount;
        SCH_AppData.HkPacket.MultipleSlotsCount           = SCH_AppData.MultipleSlotsCount;
        SCH_AppData.HkPacket.SameSlotCount                = SCH_AppData.SameSlotCount;
        SCH_AppData.HkPacket.BadTableDataCount            = SCH_AppData.BadTableDataCount;
        SCH_AppData.HkPacket.TableVerifySuccessCount      = SCH_AppData.TableVerifySuccessCount;
        SCH_AppData.HkPacket.TableVerifyFailureCount      = SCH_AppData.TableVerifyFailureCount;
        SCH_AppData.HkPacket.TablePassCount               = SCH_AppData.TablePassCount;
        SCH_AppData.HkPacket.ValidMajorFrameCount         = SCH_AppData.ValidMajorFrameCount;
        SCH_AppData.HkPacket.MissedMajorFrameCount        = SCH_AppData.MissedMajorFrameCount;
        SCH_AppData.HkPacket.UnexpectedMajorFrameCount    = SCH_AppData.UnexpectedMajorFrameCount;
        SCH_AppData.HkPacket.MinorFramesSinceTone         = SCH_AppData.MinorFramesSinceTone;
        SCH_AppData.HkPacket.NextSlotNumber               = SCH_AppData.NextSlotNumber;
        SCH_AppData.HkPacket.LastSyncMETSlot              = SCH_AppData.LastSyncMETSlot;
        SCH_AppData.HkPacket.IgnoreMajorFrame             = SCH_AppData.IgnoreMajorFrame;
        SCH_AppData.HkPacket.UnexpectedMajorFrame         = SCH_AppData.UnexpectedMajorFrame;
        SCH_AppData.HkPacket.SyncToMET                    = SCH_AppData.SyncToMET;
        SCH_AppData.HkPacket.MajorFrameSource             = SCH_AppData.MajorFrameSource;
        
        /*
        ** Timestamps and send housekeeping packet
        */
        CFE_SB_TimeStampMsg((CFE_MSG_Message_t *)&SCH_AppData.HkPacket);
        CFE_SB_TransmitMsg((CFE_MSG_Message_t *)&SCH_AppData.HkPacket, true);

        /*
        ** Reset "high rate" event filters
        */
        CFE_EVS_ResetAllFilters();
    }

    /*
    ** Note:
    **
    **   The following table functions will give the cFE Table Manager
    **   a chance to update the tables used by this application.  If
    **   there is an error (very unlikely) the return value will cause
    **   us to fall out of the main process loop and terminate the SCH
    **   task.  It may sound extreme but there is nothing for the
    **   Scheduler to do if it cannot access both the message
    **   and schedule tables.
    */
    CFE_TBL_ReleaseAddress(SCH_AppData.ScheduleTableHandle);
    CFE_TBL_ReleaseAddress(SCH_AppData.MessageTableHandle);

    TableResult = SCH_AcquirePointers();

    return(TableResult);

} /* End of SCH_HousekeepingCmd() */


/*******************************************************************
**
** SCH_NoopCmd
**
** NOTE: For complete prolog information, see above
********************************************************************/

void SCH_NoopCmd(CFE_MSG_Message_t *MessagePtr)
{
    if (SCH_VerifyCmdLength(MessagePtr, sizeof(SCH_NoArgsCmd_t)) != SCH_SUCCESS)
    {
        SCH_AppData.ErrCounter++;
    }
    else
    {
        /*
        ** This command is used primarily for "aliveness" testing
        */
        SCH_AppData.CmdCounter++;

        CFE_EVS_SendEvent(SCH_NOOP_CMD_EID,
                          CFE_EVS_EventType_INFORMATION,
                          "NO-op command. Version %d.%d.%d.%d",
                          SCH_MAJOR_VERSION,
                          SCH_MINOR_VERSION,
                          SCH_REVISION,
                          SCH_MISSION_REV);
    }

    return;

} /* End of SCH_NoopCmd() */


/*******************************************************************
**
** SCH_ResetCmd
**
** NOTE: For complete prolog information, see above
********************************************************************/

void SCH_ResetCmd(CFE_MSG_Message_t *MessagePtr)
{
    if (SCH_VerifyCmdLength(MessagePtr, sizeof(SCH_NoArgsCmd_t)) != SCH_SUCCESS)
    {
        SCH_AppData.ErrCounter++;
    }
    else
    {
        /*
        ** Reset housekeeping counters
        */
        SCH_AppData.CmdCounter      = 0;
        SCH_AppData.ErrCounter      = 0;

        SCH_AppData.ScheduleActivitySuccessCount = 0;
        SCH_AppData.ScheduleActivityFailureCount = 0;

        SCH_AppData.SlotsProcessedCount = 0;
        SCH_AppData.SkippedSlotsCount   = 0;
        SCH_AppData.MultipleSlotsCount  = 0;
        SCH_AppData.SameSlotCount       = 0;
        SCH_AppData.BadTableDataCount   = 0;

        SCH_AppData.TableVerifySuccessCount = 0;
        SCH_AppData.TableVerifyFailureCount = 0;
        
        SCH_AppData.ValidMajorFrameCount      = 0;
        SCH_AppData.MissedMajorFrameCount     = 0;
        SCH_AppData.UnexpectedMajorFrameCount = 0;

        CFE_EVS_SendEvent(SCH_RESET_CMD_EID, CFE_EVS_EventType_DEBUG, "RESET command");
    }

    return;

} /* End of SCH_ResetCmd() */


/*******************************************************************
**
** SCH_EnableCmd
**
** NOTE: For complete prolog information, see above
********************************************************************/

void SCH_EnableCmd(CFE_MSG_Message_t *MessagePtr)
{
    bool         GoodCommand = false;
    SCH_EntryCmd_t *EnableCmd   = NULL;
    uint16          SlotNumber  = 0;
    uint16          EntryNumber = 0;
    uint16          TableIndex  = 0;

    /*
    ** Extract contents of command
    */
    EnableCmd   = (SCH_EntryCmd_t *) MessagePtr;

    if(SCH_VerifyCmdLength(MessagePtr, sizeof(SCH_EntryCmd_t)) == SCH_SUCCESS)
    {
    {
        SlotNumber  = EnableCmd->SlotNumber;
        EntryNumber = EnableCmd->EntryNumber;
        TableIndex  = (SlotNumber * SCH_ENTRIES_PER_SLOT) + EntryNumber;

        if ((SlotNumber >= SCH_TOTAL_SLOTS) || (EntryNumber >= SCH_ENTRIES_PER_SLOT))
        {
            /*
            ** Invalid command packet argument
            */
            CFE_EVS_SendEvent(SCH_ENABLE_CMD_ARG_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "ENABLE cmd: invalid argument, slot=%d (<%d), entry=%d (<%d)",
                              SlotNumber,
                              SCH_TOTAL_SLOTS,
                              EntryNumber,
                              SCH_ENTRIES_PER_SLOT);
        }
        else if ((SCH_AppData.ScheduleTable[TableIndex].EnableState != SCH_ENABLED) &&
                 (SCH_AppData.ScheduleTable[TableIndex].EnableState != SCH_DISABLED))
        {
            /*
            ** Invalid schedule table enable state (unused or corrupt)
            */
            CFE_EVS_SendEvent(SCH_ENABLE_CMD_ENTRY_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "ENABLE command: invalid state = %d, slot = %d, entry = %d",
                              SCH_AppData.ScheduleTable[TableIndex].EnableState,
                              SlotNumber,
                              EntryNumber);
        }
        else
        {
            /*
            ** Success
            */
            GoodCommand = true;

            SCH_AppData.ScheduleTable[TableIndex].EnableState = SCH_ENABLED;
            CFE_TBL_Modified(SCH_AppData.ScheduleTableHandle);

            CFE_EVS_SendEvent(SCH_ENABLE_CMD_EID,
                              CFE_EVS_EventType_DEBUG,
                              "ENABLE command: slot = %d, entry = %d",
                              SlotNumber,
                              EntryNumber);
        }
    }

    SCH_PostCommandResult(GoodCommand);

    return;

} /* End of SCH_EnableCmd() */


/*******************************************************************
**
** SCH_DisableCmd
**
** NOTE: For complete prolog information, see above
********************************************************************/

void SCH_DisableCmd(CFE_MSG_Message_t *MessagePtr)
{
    bool         GoodCommand = false;
    SCH_EntryCmd_t *DisableCmd  = NULL;
    uint16          SlotNumber  = 0;
    uint16          EntryNumber = 0;
    uint16          TableIndex  = 0;

    /*
    ** Extract contents of command
    */
    DisableCmd  = (SCH_EntryCmd_t *) MessagePtr;

    if (SCH_VerifyCmdLength(MessagePtr, sizeof(SCH_EntryCmd_t)) == SCH_SUCCESS)
    {
        SlotNumber  = DisableCmd->SlotNumber;
        EntryNumber = DisableCmd->EntryNumber;
        TableIndex  = (SlotNumber * SCH_ENTRIES_PER_SLOT) + EntryNumber;

        if ((SlotNumber >= SCH_TOTAL_SLOTS) || (EntryNumber >= SCH_ENTRIES_PER_SLOT))
        {
            /*
            ** Invalid command packet argument
            */
            CFE_EVS_SendEvent(SCH_DISABLE_CMD_ARG_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "DISABLE cmd: invalid argument, slot=%d (<%d), entry=%d (<%d)",
                              SlotNumber,
                              SCH_TOTAL_SLOTS,
                              EntryNumber,
                              SCH_ENTRIES_PER_SLOT);
        }
        else if ((SCH_AppData.ScheduleTable[TableIndex].EnableState != SCH_ENABLED) &&
                 (SCH_AppData.ScheduleTable[TableIndex].EnableState != SCH_DISABLED))
        {
            /*
            ** Invalid schedule table enable state (unused or corrupt)
            */
            CFE_EVS_SendEvent(SCH_DISABLE_CMD_ENTRY_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "DISABLE command: invalid state = %d, slot = %d, entry = %d",
                              SCH_AppData.ScheduleTable[TableIndex].EnableState,
                              SlotNumber,
                              EntryNumber);
        }
        else
        {
            /*
            ** Success
            */
            GoodCommand = true;

            SCH_AppData.ScheduleTable[TableIndex].EnableState = SCH_DISABLED;
            CFE_TBL_Modified(SCH_AppData.ScheduleTableHandle);

            CFE_EVS_SendEvent(SCH_DISABLE_CMD_EID,
                              CFE_EVS_EventType_DEBUG,
                              "DISABLE command: slot = %d, entry = %d",
                              SlotNumber,
                              EntryNumber);
        }
    }

    SCH_PostCommandResult(GoodCommand);

    return;

} /* End of SCH_DisableCmd() */


/*******************************************************************
**
** SCH_EnableGroupCmd
**
** NOTE: For complete prolog information, see above
********************************************************************/

void SCH_EnableGroupCmd(CFE_MSG_Message_t *MessagePtr)
{
    bool              GoodCommand    = false;
    uint32               TblGroupNumber = 0;
    uint32               TblMultiGroup  = 0;
    int32                LoopCount      = 0;
    int32                MatchCount     = 0;
    SCH_GroupCmd_t      *EnableCmd      = NULL;
    SCH_ScheduleEntry_t *TableEntry     = NULL;
    uint32               CmdGroupNumber = 0;
    uint32               CmdMultiGroup  = 0;

    /*
    ** Extract command parameters
    */
    EnableCmd = (SCH_GroupCmd_t *)MessagePtr;

    if (SCH_VerifyCmdLength(MessagePtr, sizeof(SCH_GroupCmd_t)) == SCH_SUCCESS)
    {
        TableEntry     = &SCH_AppData.ScheduleTable[0];
        CmdGroupNumber = EnableCmd->GroupData & SCH_GROUP_NUMBER_BIT_MASK;
        CmdMultiGroup  = EnableCmd->GroupData & SCH_MULTI_GROUP_BIT_MASK;

        if ((CmdGroupNumber == SCH_UNUSED) && (CmdMultiGroup == SCH_UNUSED))
        {
            /*
            ** No groups selected
            */
            CFE_EVS_SendEvent(SCH_ENA_GRP_CMD_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "ENABLE GROUP command: invalid argument, no groups selected");
        }
        else
        {
            /*
            ** Search entire schedule table for group members
            */
            for (LoopCount = 0; LoopCount < SCH_TABLE_ENTRIES; LoopCount++)
            {
                /*
                ** Skip unused table entries
                */
                if (TableEntry->GroupData != SCH_UNUSED)
                {
                    TblGroupNumber = TableEntry->GroupData & SCH_GROUP_NUMBER_BIT_MASK;
                    TblMultiGroup  = TableEntry->GroupData & SCH_MULTI_GROUP_BIT_MASK;
    
                    /*
                    ** Look for matching table entries
                    */
                    if (((CmdGroupNumber != SCH_UNUSED) && (CmdGroupNumber == TblGroupNumber)) ||
                        ((CmdMultiGroup & TblMultiGroup) != SCH_UNUSED))
                    {
                        MatchCount++;
                        TableEntry->EnableState = SCH_ENABLED;
                    }
                }
    
                TableEntry++;
            }

            if (MatchCount > 0)
            {
                CFE_TBL_Modified(SCH_AppData.ScheduleTableHandle);
                CFE_EVS_SendEvent(SCH_ENA_GRP_CMD_EID,
                                  CFE_EVS_EventType_DEBUG,
                                  "ENABLE GROUP command: match count = %d",
                                  (int)MatchCount);
                GoodCommand = true;
            }
            else
            {
                CFE_EVS_SendEvent(
                    SCH_ENA_GRP_NOT_FOUND_ERR_EID,
                    CFE_EVS_EventType_ERROR,
                    "ENABLE GROUP command: Neither Group %d nor Multi-Group 0x%06X found",
                    (int)(CmdGroupNumber >> 24),
                    (unsigned int)CmdMultiGroup);
            }
        }
    }

    SCH_PostCommandResult(GoodCommand);

    return;

} /* End of SCH_EnableGroupCmd() */


/*******************************************************************
**
** SCH_DisableGroupCmd
**
** NOTE: For complete prolog information, see above
********************************************************************/

void SCH_DisableGroupCmd(CFE_MSG_Message_t *MessagePtr)
{
    bool              GoodCommand    = false;
    uint32               TblGroupNumber = 0;
    uint32               TblMultiGroup = 0;
    int32                LoopCount = 0;
    int32                MatchCount = 0;
    SCH_GroupCmd_t      *DisableCmd = NULL;
    SCH_ScheduleEntry_t *TableEntry = NULL;
    uint32               CmdGroupNumber = 0;
    uint32               CmdMultiGroup  = 0;

    /*
    ** Extract command parameters
    */
    DisableCmd = (SCH_GroupCmd_t *)MessagePtr;

    if (SCH_VerifyCmdLength(MessagePtr, sizeof(SCH_GroupCmd_t)) == SCH_SUCCESS)
    {
        TableEntry     = &SCH_AppData.ScheduleTable[0];
        CmdGroupNumber = DisableCmd->GroupData & SCH_GROUP_NUMBER_BIT_MASK;
        CmdMultiGroup  = DisableCmd->GroupData & SCH_MULTI_GROUP_BIT_MASK;

        if ((CmdGroupNumber == SCH_UNUSED) && (CmdMultiGroup == SCH_UNUSED))
        {
            /*
            ** No groups selected
            */
            CFE_EVS_SendEvent(SCH_DIS_GRP_CMD_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "DISABLE GROUP command: invalid argument, no groups selected");
        }
        else
        {
            /*
            ** Search entire schedule table for group members
            */
            for (LoopCount = 0; LoopCount < SCH_TABLE_ENTRIES; LoopCount++)
            {
                /*
                ** Skip unused table entries
                */
                if (TableEntry->GroupData != SCH_UNUSED)
                {
                    TblGroupNumber = TableEntry->GroupData & SCH_GROUP_NUMBER_BIT_MASK;
                    TblMultiGroup  = TableEntry->GroupData & SCH_MULTI_GROUP_BIT_MASK;
    
                    /*
                    ** Look for matching table entries
                    */
                    if (((CmdGroupNumber != SCH_UNUSED) && (CmdGroupNumber == TblGroupNumber)) ||
                        ((CmdMultiGroup & TblMultiGroup) != SCH_UNUSED))
                    {
                        MatchCount++;
                        TableEntry->EnableState = SCH_DISABLED;
                    }
                }

                TableEntry++;
            }

            if (MatchCount > 0)
            {
                CFE_TBL_Modified(SCH_AppData.ScheduleTableHandle);
                CFE_EVS_SendEvent(SCH_DIS_GRP_CMD_EID,
                                  CFE_EVS_EventType_DEBUG,
                                  "DISABLE GROUP command: match count = %d",
                                  (int)MatchCount);
                GoodCommand = true;
            }
            else
            {
                CFE_EVS_SendEvent(
                    SCH_DIS_GRP_NOT_FOUND_ERR_EID,
                    CFE_EVS_EventType_ERROR,
                    "DISABLE GROUP command: Neither Group %d nor Multi-Group 0x%06X found",
                    (int)(CmdGroupNumber >> 24),
                    (unsigned int)CmdMultiGroup);
            }
        }
    }

    SCH_PostCommandResult(GoodCommand);

    return;

} /* End of SCH_DisableGroupCmd() */


/*******************************************************************
**
** SCH_EnableSyncCmd
**
** NOTE: For complete prolog information, see above
********************************************************************/

void SCH_EnableSyncCmd(CFE_MSG_Message_t *MessagePtr)
{
    bool GoodCommand = false;

    if (SCH_VerifyCmdLength(MessagePtr, sizeof(SCH_NoArgsCmd_t)) == SCH_SUCCESS)
    {
        GoodCommand = true;

        SCH_AppData.IgnoreMajorFrame             = false;
        SCH_AppData.UnexpectedMajorFrame         = false;
        SCH_AppData.ConsecutiveNoisyFrameCounter = 0;

        CFE_EVS_SendEvent(
            SCH_ENA_SYNC_CMD_EID, CFE_EVS_EventType_DEBUG, "Major Frame Synchronization Enabled");
    }

    SCH_PostCommandResult(GoodCommand);

    return;

} /* End of SCH_EnableSyncCmd() */




/*******************************************************************
**
** SCH_SendDiagTlmCmd
**
** NOTE: For complete prolog information, see above
********************************************************************/

void SCH_SendDiagTlmCmd(CFE_MSG_Message_t *MessagePtr)
{
    uint32               TblIndex = 0;
    uint32               WordIndex = 0;
    uint32               BitIndex = 0;
    SCH_ScheduleEntry_t *TableEntry = NULL;
    bool              GoodCommand = false;

    if (SCH_VerifyCmdLength(MessagePtr, sizeof(SCH_NoArgsCmd_t)) == SCH_SUCCESS)
    {
        GoodCommand = true;

        /* Zero out the previous entry states */
        CFE_PSP_MemSet(&SCH_AppData.DiagPacket.EntryStates[0], 0x0, SCH_NUM_STATUS_BYTES_REQD);

        for (TblIndex = 0; TblIndex < SCH_TABLE_ENTRIES; TblIndex++)
        {
            TableEntry = &SCH_AppData.ScheduleTable[TblIndex];
            WordIndex = TblIndex/8;             /* 8 states can fit in each word */
            BitIndex  = (7-(TblIndex%8))*2;     /* Determine bit pair, MSBs contain lowest index */
            
            if (TableEntry->EnableState == SCH_ENABLED)
            {
                SCH_AppData.DiagPacket.EntryStates[WordIndex] |= (1 << BitIndex);
                CFE_MSG_GetMsgId(
                    (CFE_MSG_Message_t *)&SCH_AppData
                    .MessageTable[SCH_AppData.ScheduleTable[TblIndex].MessageIndex],
                    &SCH_AppData.DiagPacket.MsgIDs[TblIndex]
                );
            }
            else if (TableEntry->EnableState == SCH_DISABLED)
            {
                SCH_AppData.DiagPacket.EntryStates[WordIndex] |= (2 << BitIndex);
                CFE_MSG_GetMsgId(
                    (CFE_MSG_Message_t *)&SCH_AppData
                        .MessageTable[SCH_AppData.ScheduleTable[TblIndex].MessageIndex],
                        &SCH_AppData.DiagPacket.MsgIDs[TblIndex]
                );
            }
            else
            {
                SCH_AppData.DiagPacket.MsgIDs[TblIndex] = CFE_SB_ValueToMsgId(0x0000);
            }
        }
        /*
        ** Timestamp and send diagnostic packet
        */
        CFE_SB_TimeStampMsg((CFE_MSG_Message_t *)&SCH_AppData.DiagPacket);
        CFE_SB_TransmitMsg((CFE_MSG_Message_t *)&SCH_AppData.DiagPacket, true);

        CFE_EVS_SendEvent(SCH_SEND_DIAG_CMD_EID, CFE_EVS_EventType_DEBUG, "Transmitting Diagnostic Message");
    }

    SCH_PostCommandResult(GoodCommand);

    return;

} /* End of SCH_SendDiagTlmCmd() */


/*******************************************************************
**
** SCH_AcquirePointers
**
** NOTE: For complete prolog information, see 'sch_cmds.h'
********************************************************************/

int32 SCH_AcquirePointers(void)
{
    int32  Result;

    /*
    ** Let cFE manage the tables
    */
    CFE_TBL_Manage(SCH_AppData.ScheduleTableHandle);
    CFE_TBL_Manage(SCH_AppData.MessageTableHandle);

    /*
    ** Get a pointer to the schedule table
    */
    Result = CFE_TBL_GetAddress((void *)&SCH_AppData.ScheduleTable, 
                                         SCH_AppData.ScheduleTableHandle);

    if (Result > CFE_SUCCESS)
    {
        /*
        ** Change warning results to indicate "success"
        */
        Result = CFE_SUCCESS;
    }

    /*
    ** Repeat the process for the message table
    */
    if (Result == CFE_SUCCESS)
    {
        Result = CFE_TBL_GetAddress((void *)&SCH_AppData.MessageTable, 
                                             SCH_AppData.MessageTableHandle);
        if (Result > CFE_SUCCESS)
        {
            Result = CFE_SUCCESS;
        }
    }

    return(Result);

} /* End of SCH_AcquirePointers() */


/*******************************************************************
**
** SCH_VerifyCmdLength
**
** NOTE: For complete prolog information, see above
********************************************************************/

int32 SCH_VerifyCmdLength (CFE_SB_MsgPtr_t MessagePtr, uint32 ExpectedLength)
{
    int32               Status = SCH_SUCCESS;
    CFE_SB_MsgId_t      MessageID = 0;
    uint16              CommandCode = 0; 
    uint16              ActualLength = 0;
   
    ActualLength  = CFE_SB_GetTotalMsgLength(MessagePtr);
      
    if (ExpectedLength != ActualLength)
    {
        MessageID   = CFE_SB_GetMsgId(MessagePtr);
        CommandCode = CFE_SB_GetCmdCode(MessagePtr);   
         
        CFE_EVS_SendEvent(SCH_CMD_LEN_ERR_EID, CFE_EVS_ERROR,
                          "Cmd Msg with Bad length Rcvd: ID = 0x%04X, CC = %d, Exp Len = %d, Len = %d",
                          MessageID, CommandCode, (int)ExpectedLength, ActualLength);

        Status = SCH_BAD_MSG_LENGTH_RC;
    }

    return Status;

} /* End of SCH_VerifyCmdLength () */


/*******************************************************************
**
** SCH_PostCommandResult
**
** NOTE: For complete prolog information, see above
********************************************************************/

void SCH_PostCommandResult(boolean GoodCommand)
{
    if (GoodCommand)
    {
        SCH_AppData.CmdCounter++;
    }
    else
    {
        SCH_AppData.ErrCounter++;
    }

    return;

} /* End of SCH_PostCommandResult() */



/************************/
/*  End of File Comment */
/************************/

