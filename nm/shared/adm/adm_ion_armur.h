/****************************************************************************
 **
 ** File Name: adm_ion_armur.h
 **
 ** Description: implementation of the ADM for ARMUR.
 **
 ** Notes: AMP-rule-based remote software update framework
 **
 ** Assumptions: TODO
 **
 ** Modification History: 
 **  YYYY-MM-DD  AUTHOR           DESCRIPTION
 **  ----------  --------------   --------------------------------------------
 **  2020-10-26  jigi             added EDD_RECORDS, CTRL_INIT.
 **  2020-08-12  jigi             initial integration of ARMUR to the nm module.
 **
 ****************************************************************************/


#ifndef ADM_ION_ARMUR_H_
#define ADM_ION_ARMUR_H_
#define _HAVE_DTN_ION_ARMUR_ADM_
#ifdef _HAVE_DTN_ION_ARMUR_ADM_

#include "../utils/nm_types.h"
#include "adm.h"

extern vec_idx_t g_dtn_ion_armur_idx[11];


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                        ADM TEMPLATE DOCUMENTATION                                         +
 * +-----------------------------------------------------------------------------------------------------------+
 *
 * ADM ROOT STRING:DTN/ION/armur
 */
#define ADM_ENUM_DTN_ION_ARMUR 16
/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                        AGENT NICKNAME DEFINITIONS                                         +
 * +-----------------------------------------------------------------------------------------------------------+
 */

/*
 * +------------------------------------------------------------------------------------------------------------+
 * |                                     DTN_ION_ARMUR META-DATA DEFINITIONS                                    +
 * +------------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |             DESCRIPTION              | TYPE  |         VALUE          |
 * +---------------------+--------------+--------------------------------------+-------+------------------------+
 * |name                 |458019014a00  |The human-readable name of the ADM.   |STR    |ion_armur               |
 * +---------------------+--------------+--------------------------------------+-------+------------------------+
 * |namespace            |458019014a01  |The namespace of the ADM              |STR    |DTN/ION/armur           |
 * +---------------------+--------------+--------------------------------------+-------+------------------------+
 * |version              |458019014a02  |The version of the ADM                |STR    |v1.0                    |
 * +---------------------+--------------+--------------------------------------+-------+------------------------+
 * |organization         |458019014a03  |The name of the issuing organization o|       |                        |
 * |                     |              |f the ADM                             |STR    |HYUMNI                  |
 * +---------------------+--------------+--------------------------------------+-------+------------------------+
 */
// "name"
#define DTN_ION_ARMUR_META_NAME 0x00
// "namespace"
#define DTN_ION_ARMUR_META_NAMESPACE 0x01
// "version"
#define DTN_ION_ARMUR_META_VERSION 0x02
// "organization"
#define DTN_ION_ARMUR_META_ORGANIZATION 0x03


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                             DTN_ION_ARMUR EXTERNALLY DEFINED DATA DEFINITIONS                             +
 * +-----------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |             DESCRIPTION              | TYPE  |
 * +---------------------+--------------+----------------------------------------------+-------+
 * |state                |458219014200  |The current state of ARMUR            |UINT   |
 * +---------------------+--------------+--------------------------------------+-------+
 * |records              |458219014201  |New line (LF) delimited list of ARMUR |UINT   |
 * |                     |              |log records in string                 |       |
 * +---------------------+--------------+--------------------------------------+-------+
 */
#define DTN_ION_ARMUR_EDD_STATE 0x00
#define DTN_ION_ARMUR_EDD_RECORDS 0x01


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                    DTN_ION_ARMUR VARIABLE DEFINITIONS                                     +
 * +-----------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |             DESCRIPTION              | TYPE  |
 * +---------------------+--------------+--------------------------------------+-------+
 */


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                     DTN_ION_ARMUR REPORT DEFINITIONS                                      +
 * +-----------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |             DESCRIPTION              | TYPE  |
 * +---------------------+--------------+--------------------------------------+-------+
 */


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                      DTN_ION_ARMUR TABLE DEFINITIONS                                      +
 * +-----------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |             DESCRIPTION              | TYPE  |
 * +---------------------+--------------+--------------------------------------+-------+
 */


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                     DTN_ION_ARMUR CONTROL DEFINITIONS                                     +
 * +-----------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |             DESCRIPTION              | TYPE  |
 * +---------------------+--------------+--------------------------------------+-------+
 * |init                 |458119014100  |Automate the full ARMUR procedures wit|       |
 * |                     |              |h a few parameters in a single control|       |
 * |                     |              |.                                     |       |
 * +---------------------+--------------+--------------------------------------+-------+
 * |start                |458119014101  |Triggered by Manager, install the down|       |
 * |			 |		|loaded archive, restart the applicable|       |
 * |			 |		| daemon programs and activate the armu|       |
 * |			 |		|r_sbr_fin to report the results.      |       |
 * +---------------------+--------------+--------------------------------------+-------+
 * |install              |458119014103  |Extract the binary archive and install|       |
 * |                     |              |the images.                           |       |
 * +---------------------+--------------+--------------------------------------+-------+
 * |restart              |458119014104  |Restart daemon applications according |       |
 * |                     |              |to the images on the restart queues fr|       |
 * |                     |              |om ARMUR VDB.                         |       |
 * +---------------------+--------------+--------------------------------------+-------+
 * |report               |458119014105  |Generate a report indicating the resul|       |
 * |			 |		|t of the remote software update (amp_a|       |
 * |			 |		|gent_ctrl_gen_rpts wrapper function) a|       |
 * |			 |		|nd/or do some postprocessing jobs for |       |
 * |			 |		|ARMUR DB/VDB.                         |       |
 * +---------------------+--------------+--------------------------------------+-------+
 */
#define DTN_ION_ARMUR_CTRL_INIT 0x00
#define DTN_ION_ARMUR_CTRL_START 0x01
#define DTN_ION_ARMUR_CTRL_INSTALL 0x02
#define DTN_ION_ARMUR_CTRL_RESTART 0x03
#define DTN_ION_ARMUR_CTRL_REPORT 0x04


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                    DTN_ION_ARMUR CONSTANT DEFINITIONS                                     +
 * +-----------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |             DESCRIPTION              | TYPE  |         VALUE         |
 * +---------------------+--------------+--------------------------------------+-------+-----------------------+
 */


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                      DTN_ION_ARMUR MACRO DEFINITIONS                                      +
 * +-----------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |             DESCRIPTION              | TYPE  |
 * +---------------------+--------------+--------------------------------------+-------+
 */


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                    DTN_ION_ARMUR OPERATOR DEFINITIONS                                     +
 * +-----------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |             DESCRIPTION              | TYPE  |
 * +---------------------+--------------+--------------------------------------+-------+
 */


/*
 * +-----------------------------------------------------------------------------------------------------------+
 * |                                      DTN_ION_ARMUR SBR DEFINITIONS                                        +
 * +-----------------------------------------------------------------------------------------------------------+
 * |        NAME         |     ARI      |             DESCRIPTION              | TYPE  |
 * +---------------------+--------------+--------------------------------------+-------+
 * |start                |458819014600  |Predefined for armur_ctrl_start proced|       |
 * |                     |              |ure at the very beginning.            |       |
 * +---------------------+--------------+--------------------------------------+-------+
 * |continue             |458819014601  |Predefined for armur_ctrl_start proced|       |
 * |                     |              |ure, especially when the node has been|       |
 * |			 |		|rebooted, to continue from the last AR|       |
 * |                     |              |MUR state.                            |       |
 * +---------------------+--------------+--------------------------------------+-------+
 * |report               |458819014602  |Predefined for armur_ctrl_report proce|       |
 * |                     |              |dure at the final stage.              |       |
 * +---------------------+--------------+--------------------------------------+-------+
 */
#define DTN_ION_ARMUR_SBR_START 0x00
#define DTN_ION_ARMUR_SBR_CONTINUE 0x01
#define DTN_ION_ARMUR_SBR_REPORT 0x02


/* Initialization functions. */
void dtn_ion_armur_init();
void dtn_ion_armur_init_meta();
void dtn_ion_armur_init_cnst();
void dtn_ion_armur_init_edd();
void dtn_ion_armur_init_op();
void dtn_ion_armur_init_var();
void dtn_ion_armur_init_ctrl();
void dtn_ion_armur_init_mac();
void dtn_ion_armur_init_rpttpl();
void dtn_ion_armur_init_tblt();
void dtn_ion_armur_init_sbr();

#endif /* _HAVE_DTN_ION_ARMUR_ADM_ */
#endif //ADM_ION_ARMUR_H_
