static int omnia_II_init_gpio_table[][6] = {
	/* GPA */
	{ GPIO_AP_FLM_RXD, GPIO_AP_FLM_RXD_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_AP_FLM_TXD, GPIO_AP_FLM_TXD_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT1, S3C_GPIO_PULL_NONE },
	
	{ GPIO_AP_BT_RXD, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN }, 
	{ GPIO_AP_BT_TXD, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN }, 
	{ GPIO_AP_BT_CTS, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN }, 
	{ GPIO_AP_BT_RTS, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN }, 

	/* GPB */
//	{ GPIO_AP_RXD, 2, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
//	{ GPIO_AP_TXD, 2, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT1, S3C_GPIO_PULL_NONE },

	{ GPIO_CAM_SCL, 6, GPIO_LEVEL_HIGH, S3C_GPIO_PULL_UP, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE }, 
	{ GPIO_CAM_SDA, 6, GPIO_LEVEL_HIGH, S3C_GPIO_PULL_UP, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE }, 
	{ GPIO_TFLASH_EN, 1, GPIO_LEVEL_HIGH, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE }, 
	{ GPIO_AP_SCL, 2, GPIO_LEVEL_HIGH, S3C_GPIO_PULL_UP, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE }, 
	{ GPIO_AP_SDA, 2, GPIO_LEVEL_HIGH, S3C_GPIO_PULL_UP, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE }, 

	/* GPC */
	{ GPIO_WLAN_SPI_RXD, 2, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN }, 
	{ GPIO_WLAN_SPI_CLK, 2, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN }, 
	{ GPIO_WLAN_SPI_TXD, 2, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN }, 
	{ GPIO_WLAN_SPI_nCS, 2, GPIO_LEVEL_LOW, S3C_GPIO_PULL_UP, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN }, 

	/* GPD */
	{ GPIO_I2S_SCLK, 3, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN }, 
	{ GPIO_I2S_SYNC, 3, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN }, 
	{ GPIO_I2S_SDI, 3, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN }, 
	{ GPIO_I2S_SDO, 3, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },

        /* GPE */
        { GPIO_BT_nRST, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },
        { GPIO_BOOT_MODE, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
        { GPIO_WLAN_nRST, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },
        { GPIO_PWR_I2C_SCL, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
        { GPIO_PWR_I2C_SDA, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },

	/* GPF */
	{ GPIO_CAM_MCLK, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_HSYNC, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_PCLK, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_3M_nRST, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },
	{ GPIO_CAM_VSYNC, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_D_0, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_D_1, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_D_2, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_D_3, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_D_4, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_D_5, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_D_6, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_D_7, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_VIBTONE_PWM, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },

	/* GPG */
	{ GPIO_T_FLASH_CLK, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_T_FLASH_CMD, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_T_FLASH_D0, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_T_FLASH_D1, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_T_FLASH_D2, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_T_FLASH_D3, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },

	/* GPH */
	{ GPIO_NAND_CLK, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_NAND_CMD, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_NAND_D0, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_NAND_D1, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_NAND_D2, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_NAND_D3, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },

	/* GPK */
	{ GPIO_CAM_EN, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_AUDIO_EN, GPIO_AUDIO_EN_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 
	{ GPIO_FM_nRST, GPIO_FM_nRST_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 
	{ GPIO_CAM_CIF_nRST, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 
	{ GPIO_CAM_CIF_nSTBY, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 
	{ GPIO_UART_SEL, GPIO_UART_SEL_AF, GPIO_LEVEL_HIGH, S3C_GPIO_PULL_NONE, 0, 0 },

	/* GPL */
	{ GPIO_CAM_3M_nSTBY, 1, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, 0, 0 },
	{ GPIO_FM_INT, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, 0, 0 }, 
	{ GPIO_VIB_EN, GPIO_VIB_EN_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 
	{ GPIO_ALS_EN, GPIO_ALS_EN_AF, GPIO_LEVEL_HIGH, S3C_GPIO_PULL_NONE, 0, 0 }, 

	/* GPM */
	{ GPIO_FM_SCL, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 }, 
	{ GPIO_FM_SDA, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 }, 

};

static int omnia_II_sleep_gpio_table[][6] = {
	/** ALIVE PART **/
	/* GPK */
	{ GPIO_CAM_EN, GPIO_CAM_EN_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 
//	{ GPIO_AUDIO_EN, GPIO_AUDIO_EN_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 
//	{ GPIO_PHONE_RST_N, GPIO_PHONE_RST_N_AF, GPIO_LEVEL_HIGH, S3C_GPIO_PULL_NONE, 0, 0 }, 
#if 1  // #if 0  Enable anykey wakeup      
	{ GPIO_KBR0, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 }, 
	{ GPIO_KBR1, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_KBR2, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 }, 
#endif
//	{ GPIO_FM_nRST, GPIO_FM_nRST_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 
//	{ GPIO_CAM_CIF_nRST, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 
//	{ GPIO_CAM_CIF_nSTBY, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 

//	{ GPIO_MSENSE_RST, GPIO_MSENSE_RST_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, // bss 
//	{ GPIO_VREG_MSMP_26V, GPIO_VREG_MSMP_26V_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 }, 

	/* GPL */
	{ GPIO_KBC0, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 
	{ GPIO_KBC1, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 
	{ GPIO_KBC1, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 
//	{ GPIO_PHONE_ON, GPIO_PHONE_ON_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 
//	{ GPIO_TA_nCONNECTED, GPIO_TA_nCONNECTED_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 }, 
//	{ GPIO_USIM_BOOT, GPIO_USIM_BOOT_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 
//	{ GPIO_CAM_3M_nSTBY, 0, GPIO_LEVEL_NONE,S3C_GPIO_PULL_NONE, 0, 0 },
//	{ GPIO_MIC_SEL, GPIO_MIC_SEL_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, // can be good
//	{ GPIO_FM_INT, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, 0, 0 }, 
//	{ GPIO_VIB_EN, GPIO_VIB_EN_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 
//	{ GPIO_ALS_EN, GPIO_ALS_EN_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 

	/* GPM */
	{ GPIO_FM_SCL, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 }, 
	{ GPIO_FM_SDA, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 }, 
//	{ GPIO_TA_nCHG, GPIO_TA_nCHG_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 }, 
	{ GPIO_PDA_ACTIVE, GPIO_PDA_ACTIVE_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 

	/* GPN */ 
//	{ GPIO_nPOWER, GPIO_nPOWER_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_UP, 0, 0 }, 

	/** MEMORY PART **/
	/* GPO to check */
	{ GPIO_LCD_RST_N, GPIO_LCD_RST_N_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },
	{ GPIO_LCD_CS_N, GPIO_LCD_CS_N_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },
	{ GPIO_LCD_SDI, GPIO_LCD_SDI_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },
	{ GPIO_LCD_ID, GPIO_LCD_ID_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	{ GPIO_LCD_SCLK, GPIO_LCD_SCLK_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },

	/* GPP */
	#define MEM0_OEata 2	//MEM0_OEata controled by MEM0CONSLP1
	{ GPIO_PS_HOLD_PDA, MEM0_OEata, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT1, S3C_GPIO_PULL_NONE }, 
};


