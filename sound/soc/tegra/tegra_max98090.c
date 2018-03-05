/*
 * tegra_max98090.c - Tegra machine ASoC driver for boards using
 * MAX98090 codec.
 *
 * Author: Kamal Kannan Balagopalan <kbalagopalan@nvidia.com>
 *
 * Copyright 2007 Wolfson Microelectronics PLC.
 * Author: Graeme Gregory
 *         graeme.gregory@wolfsonmicro.com or linux@wolfsonmicro.com
 *
 * Copyright (c) 2010-2014, NVIDIA CORPORATION. All rights reserved.
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <asm/mach-types.h>

#include <linux/clk.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#ifdef CONFIG_SWITCH
#include <linux/switch.h>
#endif

#include <mach/tegra_asoc_pdata.h>

#include <sound/core.h>
#include <sound/jack.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include "../codecs/max98090.h"
#include "../codecs/max97236.h"

#include "tegra_pcm.h"
#include "tegra_asoc_utils.h"
#ifndef CONFIG_ARCH_TEGRA_2x_SOC
#include "tegra30_ahub.h"
#include "tegra30_i2s.h"
#include "tegra30_dam.h"
#endif
#define DRV_NAME "tegra-snd-max98090"

#ifndef __devinit
#define __devinit
#endif
#ifndef __devexit
#define __devexit
#endif
#ifndef __devexit_p
#define __devexit_p(x)	(x)
#endif

#define GPIO_SPKR_EN    BIT(0)
#define GPIO_HP_MUTE    BIT(1)
#define GPIO_INT_MIC_EN BIT(2)
#define GPIO_EXT_MIC_EN BIT(3)
#define GPIO_HP_DET     BIT(4)

#define DAI_LINK_HIFI		0
#define DAI_LINK_BTSCO		1
#define DAI_LINK_VOICE_CALL	2
#define DAI_LINK_BT_VOICE_CALL	3
#define DAI_LINK_HIFI_MAX97236	4
#define DAI_LINK_PCM_OFFLOAD_FE		5
#define DAI_LINK_COMPR_OFFLOAD_FE		6
#define DAI_LINK_PCM_OFFLOAD_CAPTURE_FE 	7
#define DAI_LINK_I2S_OFFLOAD_BE		8


const char *tegra_max98090_i2s_dai_name[TEGRA30_NR_I2S_IFC] = {
	"tegra30-i2s.0",
	"tegra30-i2s.1",
	"tegra30-i2s.2",
	"tegra30-i2s.3",
	"tegra30-i2s.4",
};

struct tegra_max98090 {
	struct tegra_asoc_utils_data util_data;
	struct tegra_asoc_platform_data *pdata;
	int gpio_requested;
	bool init_done;
	int is_call_mode;
	int is_device_bt;
	struct codec_config codec_info[NUM_I2S_DEVICES];
	struct ahub_bbc1_config ahub_bbc1_info;
	struct regulator *avdd_aud_reg;
	struct regulator *vdd_sw_1v8_reg;
	enum snd_soc_bias_level bias_level;
	struct snd_soc_card *pcard;
#ifdef CONFIG_SWITCH
	int jack_status;
#endif
	int clock_enabled;
};

static int tegra_call_mode_info(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;
	return 0;
}

static int tegra_call_mode_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct tegra_max98090 *machine = snd_kcontrol_chip(kcontrol);

	ucontrol->value.integer.value[0] = machine->is_call_mode;

	return 0;
}

static int tegra_call_mode_put(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct tegra_max98090 *machine = snd_kcontrol_chip(kcontrol);
	int is_call_mode_new = ucontrol->value.integer.value[0];
	int codec_index;
	unsigned int i;

	if (machine->is_call_mode == is_call_mode_new)
		return 0;

	if (machine->is_device_bt)
		codec_index = BT_SCO;
	else
		codec_index = HIFI_CODEC;

	if (is_call_mode_new) {
		if (machine->codec_info[codec_index].rate == 0 ||
			machine->codec_info[codec_index].channels == 0)
				return -EINVAL;

		for (i = 0; i < machine->pcard->num_links; i++)
			machine->pcard->dai_link[i].ignore_suspend = 1;

#if defined(CONFIG_ARCH_TEGRA_14x_SOC)
	tegra_asoc_utils_tristate_dap(
		machine->codec_info[codec_index].i2s_id, false);
	if (machine->is_device_bt) {
		t14x_make_bt_voice_call_connections(
			&machine->codec_info[codec_index],
			&machine->ahub_bbc1_info, 0);
	} else {
		t14x_make_voice_call_connections(
			&machine->codec_info[codec_index],
			&machine->ahub_bbc1_info, 0);
	}
#else
		tegra30_make_voice_call_connections(
			&machine->codec_info[codec_index],
			&machine->codec_info[BASEBAND], 0);
#endif
	} else {
#if defined(CONFIG_ARCH_TEGRA_14x_SOC)
		if (machine->is_device_bt) {
			t14x_break_bt_voice_call_connections(
				&machine->codec_info[codec_index],
				&machine->ahub_bbc1_info, 0);
		} else {
			t14x_break_voice_call_connections(
			&machine->codec_info[codec_index],
			&machine->ahub_bbc1_info, 0);
		}
		tegra_asoc_utils_tristate_dap(
			machine->codec_info[codec_index].i2s_id, true);
#else
		tegra30_break_voice_call_connections(
			&machine->codec_info[codec_index],
			&machine->codec_info[BASEBAND], 0);
#endif

		for (i = 0; i < machine->pcard->num_links; i++)
			machine->pcard->dai_link[i].ignore_suspend = 0;
	}

	machine->is_call_mode = is_call_mode_new;
	g_is_call_mode = machine->is_call_mode;

	return 1;
}

struct snd_kcontrol_new tegra_max98090_call_mode_control = {
	.access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name = "Call Mode Switch",
	.private_value = 0xffff,
	.info = tegra_call_mode_info,
	.get = tegra_call_mode_get,
	.put = tegra_call_mode_put
};

static int tegra_max98090_set_dam_cif(int dam_ifc, int srate,
			int channels, int bit_size, int src_on, int src_srate,
			int src_channels, int src_bit_size)
{
	tegra30_dam_set_gain(dam_ifc, TEGRA30_DAM_CHIN1, 0x1000);
	tegra30_dam_set_samplerate(dam_ifc, TEGRA30_DAM_CHOUT,
				srate);
	tegra30_dam_set_samplerate(dam_ifc, TEGRA30_DAM_CHIN1,
				srate);
#ifndef CONFIG_ARCH_TEGRA_3x_SOC
	tegra30_dam_set_acif(dam_ifc, TEGRA30_DAM_CHIN1,
		channels, bit_size, channels,
				32);
	tegra30_dam_set_acif(dam_ifc, TEGRA30_DAM_CHOUT,
		channels, bit_size, channels,
				32);
#else
	tegra30_dam_set_acif(dam_ifc, TEGRA30_DAM_CHIN1,
		channels, bit_size, channels,
				bit_size);
	tegra30_dam_set_acif(dam_ifc, TEGRA30_DAM_CHOUT,
		channels, bit_size, channels,
				bit_size);
#endif

	if (src_on) {
		tegra30_dam_set_gain(dam_ifc, TEGRA30_DAM_CHIN0_SRC, 0x1000);
		tegra30_dam_set_samplerate(dam_ifc, TEGRA30_DAM_CHIN0_SRC,
			src_srate);
#ifndef CONFIG_ARCH_TEGRA_3x_SOC
		tegra30_dam_set_acif(dam_ifc, TEGRA30_DAM_CHIN0_SRC,
			src_channels, src_bit_size, 1, 32);
#else
		tegra30_dam_set_acif(dam_ifc, TEGRA30_DAM_CHIN0_SRC,
			src_channels, src_bit_size, 1, 16);
#endif
	}

	return 0;
}

static int tegra_max98090_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_card *card = codec->card;
	struct tegra_max98090 *machine = snd_soc_card_get_drvdata(card);
	struct tegra_asoc_platform_data *pdata = machine->pdata;
	struct tegra30_i2s *i2s = snd_soc_dai_get_drvdata(cpu_dai);
	int srate, mclk, sample_size, i2s_daifmt, i2s_master;
	int err;
	int rate;

	i2s_master = pdata->i2s_param[HIFI_CODEC].is_i2s_master;

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		sample_size = 16;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		sample_size = 24;
		break;
	default:
		return -EINVAL;
	}

	srate = params_rate(params);
	switch (srate) {
	case 8000:
	case 16000:
	case 24000:
	case 32000:
	case 48000:
	case 64000:
	case 96000:
		mclk = 12288000;
		break;
	case 11025:
	case 22050:
	case 44100:
	case 88200:
		mclk = 11289600;
		break;
	default:
		mclk = 12000000;
		break;
	}

	i2s_daifmt = SND_SOC_DAIFMT_NB_NF;
	i2s_daifmt |= i2s_master ? SND_SOC_DAIFMT_CBS_CFS :
				SND_SOC_DAIFMT_CBM_CFM;

	switch (pdata->i2s_param[HIFI_CODEC].i2s_mode) {
	case TEGRA_DAIFMT_I2S:
		i2s_daifmt |= SND_SOC_DAIFMT_I2S;
		break;
	case TEGRA_DAIFMT_DSP_A:
		i2s_daifmt |= SND_SOC_DAIFMT_DSP_A;
		break;
	case TEGRA_DAIFMT_DSP_B:
		i2s_daifmt |= SND_SOC_DAIFMT_DSP_B;
		break;
	case TEGRA_DAIFMT_LEFT_J:
		i2s_daifmt |= SND_SOC_DAIFMT_LEFT_J;
		break;
	case TEGRA_DAIFMT_RIGHT_J:
		i2s_daifmt |= SND_SOC_DAIFMT_RIGHT_J;
		break;
	default:
		dev_err(card->dev, "Can't configure i2s format\n");
		return -EINVAL;
	}

	err = tegra_asoc_utils_set_rate(&machine->util_data, srate, mclk);
	if (err < 0) {
		if (!(machine->util_data.set_mclk % mclk))
			mclk = machine->util_data.set_mclk;
		else {
			dev_err(card->dev, "Can't configure clocks\n");
			return err;
		}
	}

	tegra_asoc_utils_lock_clk_rate(&machine->util_data, 1);

	rate = clk_get_rate(machine->util_data.clk_cdev1);

	err = snd_soc_dai_set_fmt(cpu_dai, i2s_daifmt);
	if (err < 0) {
		dev_err(card->dev, "cpu_dai fmt not set\n");
		return err;
	}

	err = snd_soc_dai_set_sysclk(codec_dai, 0, rate, SND_SOC_CLOCK_IN);

	if (err < 0) {
		dev_err(card->dev, "codec_dai clock not set\n");
		return err;
	}

	if (pdata->i2s_param[HIFI_CODEC].i2s_mode == TEGRA_DAIFMT_I2S) {
		err = snd_soc_dai_set_fmt(codec_dai, i2s_daifmt);
		if (err < 0) {
			dev_err(card->dev, "codec_dai fmt not set\n");
			return err;
		}
	} else {
		err = snd_soc_dai_set_tdm_slot(codec_dai, 3, 3, 2, sample_size);
		if (err < 0) {
			dev_err(card->dev, "codec_dai tdm mode not set\n");
			return err;
		}
	}

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK && i2s->is_dam_used)
		tegra_max98090_set_dam_cif(i2s->dam_ifc, srate,
			params_channels(params), sample_size, 0, 0, 0, 0);

	return 0;
}

static int tegra_bt_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct tegra30_i2s *i2s = snd_soc_dai_get_drvdata(rtd->cpu_dai);
	struct snd_soc_card *card = rtd->card;
	struct tegra_max98090 *machine = snd_soc_card_get_drvdata(card);
	struct tegra_asoc_platform_data *pdata = machine->pdata;
	int err, srate, mclk, min_mclk, sample_size;
	int i2s_daifmt;

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		sample_size = 16;
		break;
	default:
		return -EINVAL;
	}

	srate = params_rate(params);
	switch (srate) {
	case 11025:
	case 22050:
	case 44100:
	case 88200:
		mclk = 11289600;
		break;
	case 8000:
	case 16000:
	case 32000:
	case 48000:
	case 64000:
	case 96000:
		mclk = 12288000;
		break;
	default:
		return -EINVAL;
	}
	min_mclk = 64 * srate;

	err = tegra_asoc_utils_set_rate(&machine->util_data, srate, mclk);
	if (err < 0) {
		if (!(machine->util_data.set_mclk % min_mclk))
			mclk = machine->util_data.set_mclk;
		else {
			dev_err(card->dev, "Can't configure clocks\n");
			return err;
		}
	}

	tegra_asoc_utils_lock_clk_rate(&machine->util_data, 1);

	i2s_daifmt = SND_SOC_DAIFMT_NB_NF;
	i2s_daifmt |= pdata->i2s_param[BT_SCO].is_i2s_master ?
			SND_SOC_DAIFMT_CBS_CFS : SND_SOC_DAIFMT_CBM_CFM;

	switch (pdata->i2s_param[BT_SCO].i2s_mode) {
	case TEGRA_DAIFMT_I2S:
		i2s_daifmt |= SND_SOC_DAIFMT_I2S;
		break;
	case TEGRA_DAIFMT_DSP_A:
		i2s_daifmt |= SND_SOC_DAIFMT_DSP_A;
		break;
	case TEGRA_DAIFMT_DSP_B:
		i2s_daifmt |= SND_SOC_DAIFMT_DSP_B;
		break;
	case TEGRA_DAIFMT_LEFT_J:
		i2s_daifmt |= SND_SOC_DAIFMT_LEFT_J;
		break;
	case TEGRA_DAIFMT_RIGHT_J:
		i2s_daifmt |= SND_SOC_DAIFMT_RIGHT_J;
		break;
	default:
		dev_err(card->dev, "Can't configure i2s format\n");
		return -EINVAL;
	}

	err = snd_soc_dai_set_fmt(rtd->cpu_dai, i2s_daifmt);
	if (err < 0) {
		dev_err(rtd->codec->card->dev, "cpu_dai fmt not set\n");
		return err;
	}

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK && i2s->is_dam_used)
		tegra_max98090_set_dam_cif(i2s->dam_ifc, params_rate(params),
			params_channels(params), sample_size, 0, 0, 0, 0);

	return 0;
}

static int tegra_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct tegra_max98090 *machine = snd_soc_card_get_drvdata(rtd->card);

	tegra_asoc_utils_lock_clk_rate(&machine->util_data, 0);

	return 0;
}

static int tegra_offload_hw_params_be_fixup(struct snd_soc_pcm_runtime *rtd,
			struct snd_pcm_hw_params *params)
{
	if (!params_rate(params)) {
		struct snd_interval *snd_rate = hw_param_interval(params,
						SNDRV_PCM_HW_PARAM_RATE);

		snd_rate->min = snd_rate->max = 48000;
	}

	if (!params_channels(params)) {
		struct snd_interval *snd_channels = hw_param_interval(params,
						SNDRV_PCM_HW_PARAM_CHANNELS);

		snd_channels->min = snd_channels->max = 2;
	}
	snd_mask_set(hw_param_mask(params, SNDRV_PCM_HW_PARAM_FORMAT),
				ffs(SNDRV_PCM_FORMAT_S16_LE));

	pr_debug("%s::%d %d %d\n", __func__, params_rate(params),
			params_channels(params), params_format(params));
	return 0;
}

static int tegra_max98090_startup(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct tegra30_i2s *i2s = snd_soc_dai_get_drvdata(cpu_dai);
	struct tegra_max98090 *machine = snd_soc_card_get_drvdata(rtd->card);
	struct codec_config *codec_info;
	struct codec_config *bb_info;
	int codec_index;

	tegra_asoc_utils_tristate_dap(i2s->id, false);

	if (!i2s->is_dam_used)
		return 0;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		/*dam configuration*/
		if (!i2s->dam_ch_refcount)
			i2s->dam_ifc = tegra30_dam_allocate_controller();
		if (i2s->dam_ifc < 0)
			return i2s->dam_ifc;

		tegra30_dam_allocate_channel(i2s->dam_ifc, TEGRA30_DAM_CHIN1);
		i2s->dam_ch_refcount++;
		tegra30_dam_enable_clock(i2s->dam_ifc);

		tegra30_ahub_set_rx_cif_source(TEGRA30_AHUB_RXCIF_DAM0_RX1 +
				(i2s->dam_ifc*2), i2s->playback_fifo_cif);

		/*
		*make the dam tx to i2s rx connection if this is the only
		*client using i2s for playback
		*/
		if (i2s->playback_ref_count == 1)
			tegra30_ahub_set_rx_cif_source(
				TEGRA30_AHUB_RXCIF_I2S0_RX0 + i2s->id,
				TEGRA30_AHUB_TXCIF_DAM0_TX0 + i2s->dam_ifc);

		/* enable the dam*/
		tegra30_dam_enable(i2s->dam_ifc, TEGRA30_DAM_ENABLE,
				TEGRA30_DAM_CHIN1);
	} else {

		i2s->is_call_mode_rec = machine->is_call_mode;

		if (!i2s->is_call_mode_rec)
			return 0;

		if (machine->is_device_bt)
			codec_index = BT_SCO;
		else
			codec_index = HIFI_CODEC;

		codec_info = &machine->codec_info[codec_index];
		bb_info = &machine->codec_info[BASEBAND];

		/* allocate a dam for voice call recording */

		i2s->call_record_dam_ifc = tegra30_dam_allocate_controller();
		if (i2s->call_record_dam_ifc < 0)
			return i2s->call_record_dam_ifc;

		tegra30_dam_allocate_channel(i2s->call_record_dam_ifc,
			TEGRA30_DAM_CHIN0_SRC);
		tegra30_dam_allocate_channel(i2s->call_record_dam_ifc,
			TEGRA30_DAM_CHIN1);
		tegra30_dam_enable_clock(i2s->call_record_dam_ifc);

		/* setup the connections for voice call record */
		tegra30_ahub_unset_rx_cif_source(i2s->capture_fifo_cif);
#if defined(CONFIG_ARCH_TEGRA_14x_SOC)
		/* configure the dam */
		tegra_max98090_set_dam_cif(i2s->call_record_dam_ifc,
			codec_info->rate, codec_info->channels,
			codec_info->bitsize, 1,
			machine->ahub_bbc1_info.rate,
			machine->ahub_bbc1_info.channels,
			machine->ahub_bbc1_info.sample_size);

		tegra30_ahub_set_rx_cif_source(TEGRA30_AHUB_RXCIF_DAM0_RX0 +
			(i2s->call_record_dam_ifc*2),
			TEGRA30_AHUB_TXCIF_BBC1_TX0);
#else
		/* configure the dam */
		tegra_max98090_set_dam_cif(i2s->call_record_dam_ifc,
			codec_info->rate, codec_info->channels,
			codec_info->bitsize, 1, bb_info->rate,
			bb_info->channels, bb_info->bitsize);

		tegra30_ahub_set_rx_cif_source(TEGRA30_AHUB_RXCIF_DAM0_RX0 +
			(i2s->call_record_dam_ifc*2),
			TEGRA30_AHUB_TXCIF_I2S0_TX0 + bb_info->i2s_id);
#endif
		tegra30_ahub_set_rx_cif_source(TEGRA30_AHUB_RXCIF_DAM0_RX1 +
			(i2s->call_record_dam_ifc*2),
			TEGRA30_AHUB_TXCIF_I2S0_TX0 + codec_info->i2s_id);
		tegra30_ahub_set_rx_cif_source(i2s->capture_fifo_cif,
			TEGRA30_AHUB_TXCIF_DAM0_TX0 +
			i2s->call_record_dam_ifc);

		/* enable the dam*/
		tegra30_dam_enable(i2s->call_record_dam_ifc,
				TEGRA30_DAM_ENABLE,
				TEGRA30_DAM_CHIN1);
		tegra30_dam_enable(i2s->call_record_dam_ifc,
				TEGRA30_DAM_ENABLE,
				TEGRA30_DAM_CHIN0_SRC);
	}

	return 0;
}

static void tegra_max98090_shutdown(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct tegra30_i2s *i2s = snd_soc_dai_get_drvdata(cpu_dai);

	tegra_asoc_utils_tristate_dap(i2s->id, true);

	if (!i2s->is_dam_used)
		return;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		/* disable the dam*/
		tegra30_dam_enable(i2s->dam_ifc, TEGRA30_DAM_DISABLE,
				TEGRA30_DAM_CHIN1);

		/* disconnect the ahub connections*/
		tegra30_ahub_unset_rx_cif_source(TEGRA30_AHUB_RXCIF_DAM0_RX1 +
					(i2s->dam_ifc*2));

		/* disable the dam and free the controller */
		tegra30_dam_disable_clock(i2s->dam_ifc);
		tegra30_dam_free_channel(i2s->dam_ifc, TEGRA30_DAM_CHIN1);
		i2s->dam_ch_refcount--;
		if (!i2s->dam_ch_refcount)
			tegra30_dam_free_controller(i2s->dam_ifc);
	 } else {

		if (!i2s->is_call_mode_rec)
			return;

		i2s->is_call_mode_rec = 0;

		/* disable the dam*/
		tegra30_dam_enable(i2s->call_record_dam_ifc,
			TEGRA30_DAM_DISABLE, TEGRA30_DAM_CHIN1);
		tegra30_dam_enable(i2s->call_record_dam_ifc,
			TEGRA30_DAM_DISABLE, TEGRA30_DAM_CHIN0_SRC);

		/* disconnect the ahub connections*/
		tegra30_ahub_unset_rx_cif_source(i2s->capture_fifo_cif);
		tegra30_ahub_unset_rx_cif_source(TEGRA30_AHUB_RXCIF_DAM0_RX0 +
			(i2s->call_record_dam_ifc*2));
		tegra30_ahub_unset_rx_cif_source(TEGRA30_AHUB_RXCIF_DAM0_RX1 +
			(i2s->call_record_dam_ifc*2));

		/* free the dam channels and dam controller */
		tegra30_dam_disable_clock(i2s->call_record_dam_ifc);
		tegra30_dam_free_channel(i2s->call_record_dam_ifc,
			TEGRA30_DAM_CHIN1);
		tegra30_dam_free_channel(i2s->call_record_dam_ifc,
			TEGRA30_DAM_CHIN0_SRC);
		tegra30_dam_free_controller(i2s->call_record_dam_ifc);
	 }

	return;
}

static int tegra_voice_call_hw_params(struct snd_pcm_substream *substream,
			struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_card *card = codec->card;
	struct tegra_max98090 *machine = snd_soc_card_get_drvdata(card);
	struct tegra_asoc_platform_data *pdata = machine->pdata;
	int srate, mclk, i2s_daifmt;
	int err, rate, sample_size;

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		sample_size = 16;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		sample_size = 24;
		break;
	default:
		return -EINVAL;
	}

	srate = params_rate(params);
	switch (srate) {
	case 8000:
	case 16000:
	case 24000:
	case 32000:
	case 48000:
	case 64000:
	case 96000:
		mclk = 12288000;
		break;
	case 11025:
	case 22050:
	case 44100:
	case 88200:
		mclk = 11289600;
		break;
	default:
		return -EINVAL;
		break;
	}

	i2s_daifmt = SND_SOC_DAIFMT_NB_NF;
	i2s_daifmt |= pdata->i2s_param[HIFI_CODEC].is_i2s_master ?
			SND_SOC_DAIFMT_CBS_CFS : SND_SOC_DAIFMT_CBM_CFM;

	switch (pdata->i2s_param[HIFI_CODEC].i2s_mode) {
	case TEGRA_DAIFMT_I2S:
		i2s_daifmt |= SND_SOC_DAIFMT_I2S;
		break;
	case TEGRA_DAIFMT_DSP_A:
		i2s_daifmt |= SND_SOC_DAIFMT_DSP_A;
		break;
	case TEGRA_DAIFMT_DSP_B:
		i2s_daifmt |= SND_SOC_DAIFMT_DSP_B;
		break;
	case TEGRA_DAIFMT_LEFT_J:
		i2s_daifmt |= SND_SOC_DAIFMT_LEFT_J;
		break;
	case TEGRA_DAIFMT_RIGHT_J:
		i2s_daifmt |= SND_SOC_DAIFMT_RIGHT_J;
		break;
	default:
		dev_err(card->dev, "Can't configure i2s format\n");
		return -EINVAL;
	}

	err = tegra_asoc_utils_set_rate(&machine->util_data, srate, mclk);
	if (err < 0) {
		if (!(machine->util_data.set_mclk % mclk))
			mclk = machine->util_data.set_mclk;
		else {
			dev_err(card->dev, "Can't configure clocks\n");
			return err;
		}
	}

	tegra_asoc_utils_lock_clk_rate(&machine->util_data, 1);

	rate = clk_get_rate(machine->util_data.clk_cdev1);

	if (pdata->i2s_param[HIFI_CODEC].i2s_mode == TEGRA_DAIFMT_I2S) {
		err = snd_soc_dai_set_fmt(codec_dai, i2s_daifmt);
		if (err < 0) {
			dev_err(card->dev, "codec_dai fmt not set\n");
			return err;
		}
	} else {
		err = snd_soc_dai_set_tdm_slot(codec_dai, 3, 3, 2, sample_size);
		if (err < 0) {
			dev_err(card->dev, "codec_dai tdm mode not set\n");
			return err;
		}
	}

	err = snd_soc_dai_set_sysclk(codec_dai, 0, rate, SND_SOC_CLOCK_IN);
	if (err < 0) {
		dev_err(card->dev, "codec_dai clock not set\n");
		return err;
	}

	/* codec configuration */
	machine->codec_info[HIFI_CODEC].rate = params_rate(params);
	machine->codec_info[HIFI_CODEC].channels = params_channels(params);

	machine->is_device_bt = 0;

	return 0;
}

static void tegra_voice_call_shutdown(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct tegra_max98090 *machine  =
			snd_soc_card_get_drvdata(rtd->codec->card);

	machine->codec_info[HIFI_CODEC].rate = 0;
	machine->codec_info[HIFI_CODEC].channels = 0;

	return;
}

static int tegra_bt_voice_call_hw_params(struct snd_pcm_substream *substream,
			struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_card *card = rtd->card;
	struct tegra_max98090 *machine = snd_soc_card_get_drvdata(card);
	int err, srate, mclk, min_mclk;

	srate = params_rate(params);
	switch (srate) {
	case 11025:
	case 22050:
	case 44100:
	case 88200:
		mclk = 11289600;
		break;
	case 8000:
	case 16000:
	case 32000:
	case 48000:
	case 64000:
	case 96000:
		mclk = 12288000;
		break;
	default:
		return -EINVAL;
	}
	min_mclk = 64 * srate;

	err = tegra_asoc_utils_set_rate(&machine->util_data, srate, mclk);
	if (err < 0) {
		if (!(machine->util_data.set_mclk % min_mclk))
			mclk = machine->util_data.set_mclk;
		else {
			dev_err(card->dev, "Can't configure clocks\n");
			return err;
		}
	}

	tegra_asoc_utils_lock_clk_rate(&machine->util_data, 1);

	/* codec configuration */
	machine->codec_info[BT_SCO].rate = params_rate(params);
	machine->codec_info[BT_SCO].channels = params_channels(params);

	machine->is_device_bt = 1;

	return 0;
}

static void tegra_bt_voice_call_shutdown(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct tegra_max98090 *machine  =
			snd_soc_card_get_drvdata(rtd->codec->card);

	machine->codec_info[BT_SCO].rate = 0;
	machine->codec_info[BT_SCO].channels = 0;

	return;
}

static struct snd_soc_ops tegra_max98090_ops = {
	.hw_params = tegra_max98090_hw_params,
	.hw_free = tegra_hw_free,
	.startup = tegra_max98090_startup,
	.shutdown = tegra_max98090_shutdown,
};

static struct snd_soc_ops tegra_voice_call_ops = {
	.hw_params = tegra_voice_call_hw_params,
	.shutdown = tegra_voice_call_shutdown,
	.hw_free = tegra_hw_free,
};

static struct snd_soc_ops tegra_bt_voice_call_ops = {
	.hw_params = tegra_bt_voice_call_hw_params,
	.shutdown = tegra_bt_voice_call_shutdown,
	.hw_free = tegra_hw_free,
};

static struct snd_soc_ops tegra_bt_ops = {
	.hw_params = tegra_bt_hw_params,
	.hw_free = tegra_hw_free,
	.startup = tegra_max98090_startup,
	.shutdown = tegra_max98090_shutdown,
};

/* Headphone jack */
struct snd_soc_jack tegra_max98090_hp_jack;

#ifdef CONFIG_SWITCH
static struct switch_dev tegra_max98090_headset_switch = {
	.name = "h2w",
};

/* These values are copied from WiredAccessoryObserver */
enum headset_state {
	BIT_NO_HEADSET = 0,
	BIT_HEADSET = (1 << 0),
	BIT_HEADSET_NO_MIC = (1 << 1),
};

/* Headphone jack detection gpios */
static struct snd_soc_jack_gpio tegra_max98090_hp_jack_gpio = {
	.name = "headphone detect",
	.report = SND_JACK_HEADPHONE,
	.debounce_time = 150,
	.invert = 1,
};

static int tegra_max98090_jack_notifier(struct notifier_block *self,
	unsigned long action, void *dev)
{
	struct snd_soc_jack *jack = dev;
	struct snd_soc_codec *codec = jack->codec;
	struct snd_soc_card *card = codec->card;
	struct tegra_max98090 *machine = snd_soc_card_get_drvdata(card);
	enum headset_state state = BIT_NO_HEADSET;

	if (jack == &tegra_max98090_hp_jack) {
		machine->jack_status &= ~SND_JACK_HEADPHONE;
		machine->jack_status |= (action & SND_JACK_HEADPHONE);
	} else {
		machine->jack_status &= ~SND_JACK_MICROPHONE;
		machine->jack_status |= (action & SND_JACK_MICROPHONE);
	}

	switch (machine->jack_status) {
	case SND_JACK_HEADPHONE:
		/* For now force headset mic mode */
		/* state = BIT_HEADSET_NO_MIC; */
		state = BIT_HEADSET;
		break;
	case SND_JACK_HEADSET:
		state = BIT_HEADSET;
		break;
	case SND_JACK_MICROPHONE:
		/* mic: would not report */
	default:
		state = BIT_NO_HEADSET;
	}

	if (action == jack->status)
		switch_set_state(&tegra_max98090_headset_switch, state);

	return NOTIFY_OK;
}

static struct notifier_block tegra_max98090_jack_detect_nb = {
	.notifier_call = tegra_max98090_jack_notifier,
};
#else
/* Headset jack detection DAPM pins */
static struct snd_soc_jack_pin tegra_max98090_hs_jack_pins[] = {
	{
		.pin = "Headset Mic",
		.mask = SND_JACK_MICROPHONE,
	},
	{
		.pin = "Headphone Jack",
		.mask = SND_JACK_HEADPHONE,
	},
};
#endif

static int tegra_max98090_event_hp(struct snd_soc_dapm_widget *w,
					struct snd_kcontrol *k, int event)
{
	struct snd_soc_dapm_context *dapm = w->dapm;
	struct snd_soc_card *card = dapm->card;
	struct tegra_max98090 *machine = snd_soc_card_get_drvdata(card);
	struct tegra_asoc_platform_data *pdata = machine->pdata;

	if (!(machine->gpio_requested & GPIO_HP_DET))
		return 0;

	gpio_set_value_cansleep(pdata->gpio_hp_det,
				!SND_SOC_DAPM_EVENT_ON(event));

	return 0;
}

static const struct snd_soc_dapm_widget tegra_max98090_dapm_widgets[] = {
	SND_SOC_DAPM_SPK("Int Spk", NULL),
	SND_SOC_DAPM_OUTPUT("Earpiece"),
	SND_SOC_DAPM_HP("Headphone Jack", tegra_max98090_event_hp),
	SND_SOC_DAPM_MIC("Mic Jack", NULL),
	SND_SOC_DAPM_INPUT("Ext Mic"),
	SND_SOC_DAPM_INPUT("Int Mic"),
	SND_SOC_DAPM_MIC("DMic Pri", NULL),
	SND_SOC_DAPM_MIC("DMic Sec", NULL),
};

static const struct snd_soc_dapm_route tegra_max98090_audio_map[] = {
	{"Int Spk", NULL, "SPKL"},
	{"Int Spk", NULL, "SPKR"},
	{"Earpiece", NULL, "RCVL"},
	{"Earpiece", NULL, "RCVR"},
	{"Headphone Jack", NULL, "HPL"},
	{"Headphone Jack", NULL, "HPR"},

	{ "MIC2", NULL, "Mic Jack" },

	{"MICBIAS", NULL, "Int Mic"},
	{"IN12", NULL, "MICBIAS"},
	{"MICBIAS", NULL, "Ext Mic"},
	{"IN34", NULL, "MICBIAS"},
	{"MICBIAS", NULL, "Mic Jack"},
	{"IN56", NULL, "MICBIAS"},
	{"DMICL", NULL, "DMic Pri"},
	{"DMICR", NULL, "DMic Pri"},
#ifdef CONFIG_ARCH_TEGRA_14x_SOC
	{"DMIC3", NULL, "DMic Sec"},
	{"DMIC4", NULL, "DMic Sec"},
#endif

	/* AHUB BE connections */
	{"HiFi Playback", NULL, "I2S1_OUT"},
};

static const struct snd_kcontrol_new tegra_max98090_controls[] = {
	SOC_DAPM_PIN_SWITCH("Int Spk"),
	SOC_DAPM_PIN_SWITCH("Earpiece"),
	SOC_DAPM_PIN_SWITCH("Headphone Jack"),
	SOC_DAPM_PIN_SWITCH("Mic Jack"),
	SOC_DAPM_PIN_SWITCH("Int Mic"),
	SOC_DAPM_PIN_SWITCH("Ext Mic"),
	SOC_DAPM_PIN_SWITCH("DMic Pri"),
#ifdef CONFIG_ARCH_TEGRA_14x_SOC
	SOC_DAPM_PIN_SWITCH("DMic Sec"),
#endif
};

static int tegra_max98090_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_dapm_context *dapm = &codec->dapm;
	struct snd_soc_card *card = codec->card;
	struct tegra_max98090 *machine = snd_soc_card_get_drvdata(card);
	struct tegra_asoc_platform_data *pdata = machine->pdata;
	struct tegra30_i2s *i2s = snd_soc_dai_get_drvdata(rtd->cpu_dai);
	int ret;

	i2s->is_dam_used = false;
	if (i2s->id == machine->codec_info[BT_SCO].i2s_id)
		i2s->is_dam_used = true;

	if (machine->init_done)
		return 0;

	machine->init_done = true;

	if (gpio_is_valid(pdata->gpio_hp_mute)) {
		ret = gpio_request(pdata->gpio_hp_mute, "hp_mute");
		if (ret) {
			dev_err(card->dev, "cannot get hp_mute gpio\n");
			return ret;
		}
		machine->gpio_requested |= GPIO_HP_MUTE;

		gpio_direction_output(pdata->gpio_hp_mute, 0);
	}

	if (gpio_is_valid(pdata->gpio_hp_det) &&
		of_machine_is_compatible("nvidia,norrin")) {
		tegra_max98090_hp_jack_gpio.gpio = pdata->gpio_hp_det;
		tegra_max98090_hp_jack_gpio.invert =
			!pdata->gpio_hp_det_active_high;
		snd_soc_jack_new(codec, "Headphone Jack", SND_JACK_HEADPHONE,
					&tegra_max98090_hp_jack);
#ifndef CONFIG_SWITCH
		snd_soc_jack_add_pins(&tegra_max98090_hp_jack,
					ARRAY_SIZE(tegra_max98090_hs_jack_pins),
					tegra_max98090_hs_jack_pins);
#else
		snd_soc_jack_notifier_register(&tegra_max98090_hp_jack,
						&tegra_max98090_jack_detect_nb);
#endif
		snd_soc_jack_add_gpios(&tegra_max98090_hp_jack,
					1, &tegra_max98090_hp_jack_gpio);
		machine->gpio_requested |= GPIO_HP_DET;
	}

	/* Add call mode switch control */
	ret = snd_ctl_add(codec->card->snd_card,
		snd_ctl_new1(&tegra_max98090_call_mode_control, machine));
	if (ret < 0)
		return ret;

	ret = tegra_asoc_utils_register_ctls(&machine->util_data);
	if (ret < 0)
		return ret;

	snd_soc_dapm_sync(dapm);
	return 0;
}

static struct snd_soc_dai_link tegra_max98090_dai[] = {
	[DAI_LINK_HIFI] = {
			.name = "MAX98090",
			.stream_name = "MAX98090 HIFI",
			.codec_name = "max98090.5-0010",
			.codec_dai_name = "HiFi",
			.init = tegra_max98090_init,
			.ops = &tegra_max98090_ops,
		},
	[DAI_LINK_BTSCO] = {
			.name = "BT SCO",
			.stream_name = "BT SCO PCM",
			.codec_name = "spdif-dit.1",
			.codec_dai_name = "dit-hifi",
			.init = tegra_max98090_init,
			.ops = &tegra_bt_ops,
		},
	[DAI_LINK_VOICE_CALL] = {
			.name = "VOICE CALL",
			.stream_name = "VOICE CALL PCM",
			.codec_name = "max98090.5-0010",
			.cpu_dai_name = "dit-hifi",
			.codec_dai_name = "HiFi",
			.ops = &tegra_voice_call_ops,
		},
	[DAI_LINK_BT_VOICE_CALL] = {
			.name = "BT VOICE CALL",
			.stream_name = "BT VOICE CALL PCM",
			.codec_name = "spdif-dit.2",
			.cpu_dai_name = "dit-hifi",
			.codec_dai_name = "dit-hifi",
			.ops = &tegra_bt_voice_call_ops,
		},
	[DAI_LINK_HIFI_MAX97236] {
		.name = "MAX97236",
		.stream_name = "MAX97236 HIFI",
		.codec_name = "max97236.5-0040",
		.platform_name = "tegra-pcm-audio",
		.codec_dai_name = "max97236-HiFi",
		.ops = NULL,
	},
	[DAI_LINK_PCM_OFFLOAD_FE] = {
		.name = "offload-pcm",
		.stream_name = "offload-pcm",

		.platform_name = "tegra-offload",
		.cpu_dai_name = "tegra-offload-pcm",

		.codec_dai_name =  "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",

		.dynamic = 1,
	},
	[DAI_LINK_COMPR_OFFLOAD_FE] = {
		.name = "offload-compr",
		.stream_name = "offload-compr",

		.platform_name = "tegra-offload",
		.cpu_dai_name = "tegra-offload-compr",

		.codec_dai_name =  "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",

		.dynamic = 1,
	},
	[DAI_LINK_PCM_OFFLOAD_CAPTURE_FE] = {
		.name = "offload-pcm-capture",
		.stream_name = "offload-pcm-capture",

		.platform_name = "tegra-offload",
		.cpu_dai_name = "tegra-offload-pcm",

		.codec_dai_name =  "snd-soc-dummy-dai",
		.codec_name = "snd-soc-dummy",
	},
	[DAI_LINK_I2S_OFFLOAD_BE] = {
		.name = "offload-audio-codec",
		.stream_name = "offload-audio-pcm",
		.codec_name = "max98090.0-0010",
		.codec_dai_name = "HiFi",
		.platform_name = "tegra30-i2s.1",
		.cpu_dai_name = "tegra30-i2s.1",
		.ops = &tegra_max98090_ops,

		.no_pcm = 1,

		.be_id = 0,
		.be_hw_params_fixup = tegra_offload_hw_params_be_fixup,
	},
};

static int tegra_max98090_suspend_post(struct snd_soc_card *card)
{
	struct snd_soc_jack_gpio *gpio = &tegra_max98090_hp_jack_gpio;
	struct tegra_max98090 *machine = snd_soc_card_get_drvdata(card);
	int i, suspend_allowed = 1;

	for (i = 0; i < machine->pcard->num_links; i++) {
		if (machine->pcard->dai_link[i].ignore_suspend) {
			suspend_allowed = 0;
			break;
		}
	}

	if (suspend_allowed) {
		if (gpio_is_valid(gpio->gpio))
			disable_irq(gpio_to_irq(gpio->gpio));

		if (machine->clock_enabled) {
			machine->clock_enabled = 0;
			tegra_asoc_utils_clk_disable(&machine->util_data);
		}

		if (machine->avdd_aud_reg)
			regulator_disable(machine->avdd_aud_reg);
		if (machine->vdd_sw_1v8_reg)
			regulator_disable(machine->vdd_sw_1v8_reg);
	}

	return 0;
}

static int tegra_max98090_resume_pre(struct snd_soc_card *card)
{
	int val;
	struct snd_soc_jack_gpio *gpio = &tegra_max98090_hp_jack_gpio;
	struct tegra_max98090 *machine = snd_soc_card_get_drvdata(card);
	int i, suspend_allowed = 1;
	int ret;

	for (i = 0; i < machine->pcard->num_links; i++) {
		if (machine->pcard->dai_link[i].ignore_suspend) {
			suspend_allowed = 0;
			break;
		}
	}

	if (suspend_allowed) {
		if (gpio_is_valid(gpio->gpio)) {
			val = gpio_get_value(gpio->gpio);
			val = gpio->invert ? !val : val;
			snd_soc_jack_report(gpio->jack, val, gpio->report);
			enable_irq(gpio_to_irq(gpio->gpio));
		}

		if (!machine->clock_enabled) {
			machine->clock_enabled = 1;
			tegra_asoc_utils_clk_enable(&machine->util_data);
		}

		if (machine->avdd_aud_reg)
			ret = regulator_enable(machine->avdd_aud_reg);
		if (machine->vdd_sw_1v8_reg)
			ret = regulator_enable(machine->vdd_sw_1v8_reg);
	}

	return 0;
}

static int tegra_max98090_set_bias_level(struct snd_soc_card *card,
					struct snd_soc_dapm_context *dapm,
					enum snd_soc_bias_level level)
{
	struct tegra_max98090 *machine = snd_soc_card_get_drvdata(card);

	if (machine->bias_level == SND_SOC_BIAS_OFF &&
		level != SND_SOC_BIAS_OFF && (!machine->clock_enabled)) {
		machine->clock_enabled = 1;
		machine->bias_level = level;
	}

	return 0;
}

static int tegra_max98090_set_bias_level_post(struct snd_soc_card *card,
					struct snd_soc_dapm_context *dapm,
					enum snd_soc_bias_level level)
{
	struct tegra_max98090 *machine = snd_soc_card_get_drvdata(card);

	if (machine->bias_level != SND_SOC_BIAS_OFF &&
		level == SND_SOC_BIAS_OFF && (machine->clock_enabled)) {
		machine->clock_enabled = 0;
	}

	machine->bias_level = level;

	return 0 ;
}

static int tegra_late_probe(struct snd_soc_card *card)
{
	struct device_node *np = card->dev->of_node;
	struct snd_soc_codec *codec236 =
				card->rtd[DAI_LINK_HIFI_MAX97236].codec;
	int ret;

	if (of_machine_is_compatible("nvidia,norrin"))
		return 0;

	if (of_device_is_compatible(np, "nvidia,max97236")) {
		ret = snd_soc_jack_new(codec236,
				"Headphone Jack",
				SND_JACK_HEADSET | SND_JACK_LINEOUT | 0x7E00,
				&tegra_max98090_hp_jack);
		if (ret) {
			dev_err(codec236->dev,
				"snd_soc_jack_new returned %d\n", ret);
			return ret;
		}
	}

#ifdef CONFIG_SWITCH
	snd_soc_jack_notifier_register(&tegra_max98090_hp_jack,
			&tegra_max98090_jack_detect_nb);
#else /*gpio based headset detection*/
	snd_soc_jack_add_pins(&tegra_max98090_hp_jack,
			ARRAY_SIZE(tegra_max98090_hs_jack_pins),
			tegra_max98090_hs_jack_pins);
#endif

	if (of_device_is_compatible(np, "nvidia,max97236"))
		max97236_mic_detect(codec236, &tegra_max98090_hp_jack);

	return 0;
}

static struct snd_soc_card snd_soc_tegra_max98090 = {
	.name = "tegra-max98090",
	.owner = THIS_MODULE,
	.dai_link = tegra_max98090_dai,
	.num_links = ARRAY_SIZE(tegra_max98090_dai),
	.suspend_post = tegra_max98090_suspend_post,
	.resume_pre = tegra_max98090_resume_pre,
	.set_bias_level = tegra_max98090_set_bias_level,
	.set_bias_level_post = tegra_max98090_set_bias_level_post,

	.controls = tegra_max98090_controls,
	.num_controls = ARRAY_SIZE(tegra_max98090_controls),
	.dapm_widgets = tegra_max98090_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(tegra_max98090_dapm_widgets),
	.dapm_routes = tegra_max98090_audio_map,
	.num_dapm_routes = ARRAY_SIZE(tegra_max98090_audio_map),
	.fully_routed = true,

	.late_probe	= tegra_late_probe,
};

/*
 * DT provides platform_data
 */
static struct tegra_asoc_platform_data *
	tegra_max98090_get_dt_data(struct device *dev)
{
	struct tegra_asoc_platform_data *pdata;
	struct device_node *np = NULL;

	np = dev->of_node;
	if (!np)
		return ERR_PTR(-ENODEV);

	dev_info(dev, "Platform data supplied via DT\n");

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		dev_err(dev, "Failed to allocate pdata\n");
		return ERR_PTR(-ENOMEM);
	}

	of_property_read_string(np, "nvidia,codec-name",
				&pdata->codec_name);
	of_property_read_string(np, "nvidia,codec-dai-name",
				&pdata->codec_dai_name);
	of_property_read_u32(np, "nvidia,gpio-hp-det",
				&pdata->gpio_hp_det);
	of_property_read_u32(np, "nvidia,num-links",
				&pdata->num_links);
	pdata->edp_support =
		of_property_read_bool(np, "nvidia,edp-support");
	of_property_read_u32_array(np, "nvidia,edp-states",
			pdata->edp_states, ARRAY_SIZE(pdata->edp_states));

	of_property_read_u32(np, "nvidia,i2s-param,0,audio-port-id",
				&pdata->i2s_param[HIFI_CODEC].audio_port_id);
	of_property_read_u32(np, "nvidia,i2s-param,0,is-i2s-master",
				&pdata->i2s_param[HIFI_CODEC].is_i2s_master);
	of_property_read_u32(np, "nvidia,i2s-param,0,i2s-mode",
				&pdata->i2s_param[HIFI_CODEC].i2s_mode);
	of_property_read_u32(np, "nvidia,i2s-param,0,sample-size",
				&pdata->i2s_param[HIFI_CODEC].sample_size);
	of_property_read_u32(np, "nvidia,i2s-param,0,channels",
				&pdata->i2s_param[HIFI_CODEC].channels);
	of_property_read_u32(np, "nvidia,i2s-param,0,bit-clk",
				&pdata->i2s_param[HIFI_CODEC].bit_clk);

	return pdata;
}

static struct of_device_id tegra_max98090_of_match[] = {
	{ .compatible = "nvidia,tegra-snd-max98090", },
	{ },
};
MODULE_DEVICE_TABLE(of, tegra_max98090_of_match);

static int tegra_max98090_driver_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &snd_soc_tegra_max98090;
	struct tegra_max98090 *machine;
	struct tegra_asoc_platform_data *pdata;
	int ret, i;

	dev_info(&pdev->dev, "tegra_max98090_driver_probe\n");

	if (of_machine_is_compatible("nvidia,norrin")) {
		card->num_links = ARRAY_SIZE(tegra_max98090_dai);

		card->dai_link[DAI_LINK_HIFI_MAX97236].codec_name =
				"snd-soc-dummy";
		card->dai_link[DAI_LINK_HIFI_MAX97236].codec_dai_name =
				"snd-soc-dummy-dai";
	}

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		pdata = tegra_max98090_get_dt_data(&pdev->dev);
		if (IS_ERR(pdata))
			return PTR_ERR(pdata);
	}

	if (pdata->num_links)
		card->num_links = pdata->num_links;

	if (pdata->codec_name) {
		card->dai_link[DAI_LINK_HIFI].codec_name = pdata->codec_name;
		card->dai_link->codec_name = pdata->codec_name;
		card->dai_link[DAI_LINK_I2S_OFFLOAD_BE].codec_name =
			pdata->codec_name;
		card->dai_link[DAI_LINK_VOICE_CALL].codec_name =
			pdata->codec_name;
	}

	if (pdata->codec_dai_name) {
		card->dai_link->codec_dai_name = pdata->codec_dai_name;
		card->dai_link[DAI_LINK_I2S_OFFLOAD_BE].codec_dai_name =
			pdata->codec_dai_name;
		card->dai_link[DAI_LINK_VOICE_CALL].codec_dai_name =
			pdata->codec_dai_name;
	}

	machine = kzalloc(sizeof(struct tegra_max98090), GFP_KERNEL);
	if (!machine) {
		dev_err(&pdev->dev, "Can't allocate tegra_max98090 struct\n");
		ret = -ENOMEM;
		goto err_free_pdata;
	}

	machine->pdata = pdata;
	machine->pcard = card;
	machine->bias_level = SND_SOC_BIAS_STANDBY;
	machine->clock_enabled = 1;

	ret = tegra_asoc_utils_init(&machine->util_data, &pdev->dev, card);
	if (ret)
		goto err_free_machine;

	machine->avdd_aud_reg = regulator_get(&pdev->dev, "avdd-aud");
	if (IS_ERR(machine->avdd_aud_reg)) {
		dev_info(&pdev->dev, "avdd_aud regulator not found\n");
		machine->avdd_aud_reg = 0;
	}

	machine->vdd_sw_1v8_reg = regulator_get(&pdev->dev, "vdd-aud-dgtl");
	if (IS_ERR(machine->vdd_sw_1v8_reg)) {
		dev_info(&pdev->dev, "vdd_aud_dgtl regulator not found\n");
		machine->vdd_sw_1v8_reg = 0;
	}

	if (machine->vdd_sw_1v8_reg)
		ret = regulator_enable(machine->vdd_sw_1v8_reg);
	if (machine->avdd_aud_reg)
		ret = regulator_enable(machine->avdd_aud_reg);


#ifdef CONFIG_SWITCH
	/* Add h2w switch class support */
	ret = tegra_asoc_switch_register(&tegra_max98090_headset_switch);
	if (ret < 0) {
		dev_err(&pdev->dev, "not able to register switch device\n");
		goto err_fini_utils;
	}
#endif

	card->dev = &pdev->dev;
	platform_set_drvdata(pdev, card);
	snd_soc_card_set_drvdata(card, machine);

	for (i = 0; i < NUM_I2S_DEVICES ; i++) {
		machine->codec_info[i].i2s_id =
			pdata->i2s_param[i].audio_port_id;
		machine->codec_info[i].bitsize =
			pdata->i2s_param[i].sample_size;
		machine->codec_info[i].is_i2smaster =
			pdata->i2s_param[i].is_i2s_master;
		machine->codec_info[i].rate =
			pdata->i2s_param[i].rate;
		machine->codec_info[i].channels =
			pdata->i2s_param[i].channels;
		machine->codec_info[i].i2s_mode =
			pdata->i2s_param[i].i2s_mode;
		machine->codec_info[i].bit_clk =
			pdata->i2s_param[i].bit_clk;
	}

	if (pdata->ahub_bbc1_param) {
		machine->ahub_bbc1_info.port_id =
			pdata->ahub_bbc1_param->port_id;
		machine->ahub_bbc1_info.sample_size =
			pdata->ahub_bbc1_param->sample_size;
		machine->ahub_bbc1_info.rate =
			pdata->ahub_bbc1_param->rate;
		machine->ahub_bbc1_info.channels =
			pdata->ahub_bbc1_param->channels;
		machine->ahub_bbc1_info.bit_clk =
			pdata->ahub_bbc1_param->bit_clk;
	}

	tegra_max98090_dai[DAI_LINK_HIFI].cpu_dai_name =
	tegra_max98090_i2s_dai_name[machine->codec_info[HIFI_CODEC].i2s_id];
	tegra_max98090_dai[DAI_LINK_HIFI].platform_name =
	tegra_max98090_i2s_dai_name[machine->codec_info[HIFI_CODEC].i2s_id];

	tegra_max98090_dai[DAI_LINK_HIFI_MAX97236].cpu_dai_name =
	tegra_max98090_i2s_dai_name[machine->codec_info[HIFI_CODEC].i2s_id];
	tegra_max98090_dai[DAI_LINK_HIFI_MAX97236].platform_name =
	tegra_max98090_i2s_dai_name[machine->codec_info[HIFI_CODEC].i2s_id];

	tegra_max98090_dai[DAI_LINK_BTSCO].cpu_dai_name =
	tegra_max98090_i2s_dai_name[machine->codec_info[BT_SCO].i2s_id];
	tegra_max98090_dai[DAI_LINK_BTSCO].platform_name =
	tegra_max98090_i2s_dai_name[machine->codec_info[BT_SCO].i2s_id];

	tegra_max98090_dai[DAI_LINK_VOICE_CALL].platform_name =
	tegra_max98090_i2s_dai_name[machine->codec_info[HIFI_CODEC].i2s_id];
	tegra_max98090_dai[DAI_LINK_BT_VOICE_CALL].platform_name =
	tegra_max98090_i2s_dai_name[machine->codec_info[BT_SCO].i2s_id];

	tegra_max98090_dai[DAI_LINK_I2S_OFFLOAD_BE].cpu_dai_name =
	tegra_max98090_i2s_dai_name[machine->codec_info[HIFI_CODEC].i2s_id];

	card->dapm.idle_bias_off = 1;

	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card failed (%d)\n",
			ret);
		goto err_switch_unregister;
	}

	if (!card->instantiated) {
		dev_err(&pdev->dev, "No MAX98090 codec\n");
		goto err_unregister_card;
	}

	ret = tegra_asoc_utils_set_parent(&machine->util_data,
				pdata->i2s_param[HIFI_CODEC].is_i2s_master);
	if (ret) {
		dev_err(&pdev->dev,
				"tegra_asoc_utils_set_parent failed (%d)\n",
				ret);
		goto err_unregister_card;
	}

	return 0;

err_unregister_card:
	snd_soc_unregister_card(card);
err_switch_unregister:
#ifdef CONFIG_SWITCH
	switch_dev_unregister(&tegra_max98090_headset_switch);
#endif
err_fini_utils:
	tegra_asoc_utils_fini(&machine->util_data);
err_free_machine:
	kfree(machine);
err_free_pdata:
	if (!pdev->dev.platform_data)
		kfree(pdata);

	return ret;
}

static int __devexit tegra_max98090_driver_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	struct tegra_max98090 *machine = snd_soc_card_get_drvdata(card);
	struct tegra_asoc_platform_data *pdata = machine->pdata;

#ifdef CONFIG_SWITCH
	switch_dev_unregister(&tegra_max98090_headset_switch);
#endif

	if (machine->gpio_requested & GPIO_HP_MUTE)
		gpio_free(pdata->gpio_hp_mute);
	if (machine->gpio_requested & GPIO_HP_DET)
		snd_soc_jack_free_gpios(&tegra_max98090_hp_jack,
			1,
			&tegra_max98090_hp_jack_gpio);

	machine->gpio_requested = 0;

	if (machine->avdd_aud_reg)
		regulator_put(machine->avdd_aud_reg);

	if (machine->vdd_sw_1v8_reg)
		regulator_put(machine->vdd_sw_1v8_reg);

	snd_soc_unregister_card(card);

	tegra_asoc_utils_fini(&machine->util_data);

	kfree(machine);

	return 0;
}

static struct platform_driver tegra_max98090_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.pm = &snd_soc_pm_ops,
		.of_match_table = of_match_ptr(tegra_max98090_of_match),
	},
	.probe = tegra_max98090_driver_probe,
	.remove = __devexit_p(tegra_max98090_driver_remove),
};

static int __init tegra_max98090_modinit(void)
{
	return platform_driver_register(&tegra_max98090_driver);
}
module_init(tegra_max98090_modinit);

static void __exit tegra_max98090_modexit(void)
{
	platform_driver_unregister(&tegra_max98090_driver);
}
module_exit(tegra_max98090_modexit);

MODULE_AUTHOR("Kamal Kannan Balagopalan <kbalagopalan@nvidia.com>");
MODULE_DESCRIPTION("Tegra+MAX98090 machine ASoC driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);
