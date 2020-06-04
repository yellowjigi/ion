/****************************************************************************
 **
 ** File Name: adm_kplo_telecommand.h
 **
 ** Description: implement software management on the remote Agent node.
 **
 ** Notes: telecommand test
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-05-18  jigi		  integrated with adm_kplo_ls, adm_kplo_upgrade models
 **  2020-04-16  jigi             telecommand model v1.0
 **
 ****************************************************************************/


#ifndef ADM_KPLO_TELECOMMAND_H_
#define ADM_KPLO_TELECOMMAND_H_
#define _HAVE_DTN_KPLO_TELECOMMAND_ADM_
#ifdef _HAVE_DTN_KPLO_TELECOMMAND_ADM_

#include "../utils/nm_types.h"
#include "adm.h"

extern vec_idx_t g_dtn_kplo_telecommand_idx[11];


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                        ADM TEMPLATE DOCUMENTATION                                         +
 * +-----------------------------------------------------------------------------------------------------------+
 *
 * ADM ROOT STRING:DTN/KPLO/telecommand
 */
#define ADM_ENUM_DTN_KPLO_TELECOMMAND 13
/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                        AGENT NICKNAME DEFINITIONS                                         +
 * +-----------------------------------------------------------------------------------------------------------+
 */

/*
 * +------------------------------------------------------------------------------------------------------------+
 * |                                  DTN_KPLO_TELECOMMAND META-DATA DEFINITIONS                                +
 * +------------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |             DESCRIPTION              | TYPE  |         VALUE          |
 * +---------------------+--------------+--------------------------------------+-------+------------------------+
 * |name                 |458019012200  |The human-readable name of the ADM.   |STR    |telecommand             |
 * +---------------------+--------------+--------------------------------------+-------+------------------------+
 * |namespace            |458019012201  |The namespace of the ADM              |STR    |DTN/KPLO/telecommand    |
 * +---------------------+--------------+--------------------------------------+-------+------------------------+
 * |version              |458019012202  |The version of the ADM                |STR    |v1.0                    |
 * +---------------------+--------------+--------------------------------------+-------+------------------------+
 * |organization         |458019012203  |The name of the issuing organization o|       |                        |
 * |                     |              |f the ADM                             |STR    |HYUMNI                  |
 * +---------------------+--------------+--------------------------------------+-------+------------------------+
 */
// "name"
#define DTN_KPLO_TELECOMMAND_META_NAME 0x00
// "namespace"
#define DTN_KPLO_TELECOMMAND_META_NAMESPACE 0x01
// "version"
#define DTN_KPLO_TELECOMMAND_META_VERSION 0x02
// "organization"
#define DTN_KPLO_TELECOMMAND_META_ORGANIZATION 0x03


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                          DTN_KPLO_TELECOMMAND EXTERNALLY DEFINED DATA DEFINITIONS                         +
 * +-----------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |                   DESCRIPTION                | TYPE  |
 * +---------------------+--------------+----------------------------------------------+-------+
 */


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                 DTN_KPLO_TELECOMMAND VARIABLE DEFINITIONS                                 +
 * +-----------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |             DESCRIPTION              | TYPE  |
 * +---------------------+--------------+--------------------------------------+-------+
 */


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                  DTN_KPLO_TELECOMMAND REPORT DEFINITIONS                                  +
 * +-----------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |             DESCRIPTION              | TYPE  |
 * +---------------------+--------------+--------------------------------------+-------+
 */


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                   DTN_KPLO_TELECOMMAND TABLE DEFINITIONS                                  +
 * +-----------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |             DESCRIPTION              | TYPE  |
 * +---------------------+--------------+--------------------------------------+-------+
 */


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                  DTN_KPLO_TELECOMMAND CONTROL DEFINITIONS                                 +
 * +-----------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |             DESCRIPTION              | TYPE  |
 * +---------------------+--------------+--------------------------------------+-------+
 * |exec                 |45c119010500  |Execute a local command.              |       |
 * +---------------------+--------------+--------------------------------------+-------+
 * |kill                 |45c119010501  |Kill (or send a corresponding signal t|       |
 * |                     |              |o) a process.                         |       |
 * +---------------------+--------------+--------------------------------------+-------+
 * |update               |45c119010502  |Install given binary archives using CF|       |
 * |                     |              |DP via a (local) file server Agent and|       |
 * |                     |              |restart any applications that need to |       |
 * |                     |              |be updated.                           |       |
 * +---------------------+--------------+--------------------------------------+-------+
 */
#define DTN_KPLO_TELECOMMAND_CTRL_EXEC 0x00
#define DTN_KPLO_TELECOMMAND_CTRL_KILL 0x01
#define DTN_KPLO_TELECOMMAND_CTRL_UPDATE 0x02


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                 DTN_KPLO_TELECOMMAND CONSTANT DEFINITIONS                                 +
 * +-----------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |             DESCRIPTION              | TYPE  |         VALUE         |
 * +---------------------+--------------+--------------------------------------+-------+-----------------------+
 */


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                   DTN_KPLO_TELECOMMAND MACRO DEFINITIONS                                  +
 * +-----------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |             DESCRIPTION              | TYPE  |
 * +---------------------+--------------+--------------------------------------+-------+
 */


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                 DTN_KPLO_TELECOMMAND OPERATOR DEFINITIONS                                 +
 * +-----------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |             DESCRIPTION              | TYPE  |
 * +---------------------+--------------+--------------------------------------+-------+
 */

/* Initialization functions. */
void dtn_kplo_telecommand_init();
void dtn_kplo_telecommand_init_meta();
void dtn_kplo_telecommand_init_cnst();
void dtn_kplo_telecommand_init_edd();
void dtn_kplo_telecommand_init_op();
void dtn_kplo_telecommand_init_var();
void dtn_kplo_telecommand_init_ctrl();
void dtn_kplo_telecommand_init_mac();
void dtn_kplo_telecommand_init_rpttpl();
void dtn_kplo_telecommand_init_tblt();
#endif /* _HAVE_DTN_KPLO_TELECOMMAND_ADM_ */
#endif //ADM_KPLO_TELECOMMAND_H_
