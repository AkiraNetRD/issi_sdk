
#ifndef __FLASHBOOT_H__
#define __FLASHBOOT_H__

#define FLASH_ROOT_HEADER_ADDR 0

#define ESCAPE_CODE_MAGIC_NUM   0xEC1234EC
#define BCB_MAGIC_NUM           0xBCB56BCB
#define B1_MAGIC_NUM            0xB17890B1

typedef struct 
{
    UINT32 escape_code_magic_num;
    UINT32 escape_code_spi_offset;
    UINT32 escape_code_launch_addr;
    UINT32 BCB_magic_num;
    UINT32 BCB_flash_offset;
    UINT32 B1_magic_num;
    UINT32 B1_flash_offset;
    UINT32 B1_launch_addr;
} FLASH_root_header_t;

/**********************************************
* From system.h                               *
**********************************************/
/*! MAC address type */
#define MAC_ADDR_SIZE   6
typedef UINT8 MAC_ADDR[MAC_ADDR_SIZE];
/**********************************************
**********************************************/

typedef struct 
{
	UINT32  spi_speed;
	UINT32  disable_uart;
	UINT32  uart_baud;
	MAC_ADDR mac_addr;
} BCB_data_t_Shared; // Shared between "BCB_data_t_Golan2" and "BCB_data_t_Golan3"

typedef struct 
{
    UINT32  spi_speed;
    UINT32  disable_uart;
    UINT32  uart_baud;
    MAC_ADDR mac_addr;
    UINT16 reserved_0;
    UINT16 override_mii_regs;
    UINT16 mii_mgmt_control;
    UINT16 mii_mgmt_status;
    UINT16 mii_mgmt_phy_addr_l;  // -> mii_mgmt_reg_3
    UINT16 mii_mgmt_phy_addr_h;  // -> mii_mgmt_reg_2
    UINT16 mii_mgmt_an_adv;
    UINT16 mii_mgmt_link_partner;
    UINT16 mii_mgmt_an_expansion;
    UINT16 mii_mgmt_an_next_page;
    UINT16 mii_mgmt_an_lp_next_page;
    UINT16 mii_mgmt_ms_control;
    UINT16 mii_mgmt_ms_status;
    UINT16 mii_mgmt_extended_status;
    UINT16 pe_mcxmac_mac_config1_l;
    UINT16 pe_mcxmac_mac_config1_h;
    UINT16 pe_mcxmac_mac_config2_l;     // config2_h is not needed since it's reserved
    UINT16 pe_mcxmac_mii_mgmt_config_l;
    UINT16 pe_mcxmac_mii_mgmt_config_h;
    UINT16 pe_mcxmac_sta_addr_part1_l;
    UINT16 pe_mcxmac_sta_addr_part1_h;
    UINT16 pe_mcxmac_sta_addr_part2_l;
    UINT16 pe_mcxmac_sta_addr_part2_h;
    UINT16 emac_erx_control;
    UINT16 reserved_1;
    UINT16 override_host_mode;
    UINT16 override_phy_addr;
    UINT32 switch_reset_pulse_length;
    UINT32 launch_delay;
    UINT32 emac_phy_reset_delay;
    UINT32 emac_phy_reset_loop_cnt;
    UINT32 emac_poll_cnt;
    UINT32 spi_input_timing;
    UINT32 num_of_extra_params;
} BCB_data_t_Golan2;

typedef struct 
{
	UINT32  spi_speed;
	UINT32  disable_uart;
	UINT32  uart_baud;
	MAC_ADDR mac_addr;
	UINT16 reserved_0;
	UINT16 override_mii_regs;
	
	UINT16 reserved_1;
	UINT16 override_host_mode;
	UINT16 override_phy_addr;
	UINT32 switch_reset_pulse_length;
	UINT32 launch_delay;
	UINT32 emac_phy_reset_delay;
	UINT32 emac_phy_reset_loop_cnt;
	UINT32 emac_poll_cnt;
	UINT32 spi_input_timing;
	UINT32 num_of_extra_params;
} BCB_data_t_Golan3;

#endif // __FLASHBOOT_H__
