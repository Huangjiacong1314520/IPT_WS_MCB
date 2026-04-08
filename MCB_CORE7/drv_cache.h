/*
 * API_cache.h
 *
 *  Created on: 2020-9-30
 *      Author: ly
 */

#ifndef API_CACHE_H_
#define API_CACHE_H_

typedef enum
{
	ENUM_L1_SIZE_0_KB=0,
	ENUM_L1_SIZE_4_KB,
	ENUM_L1_SIZE_8_KB,
	ENUM_L1_SIZE_16_KB,
	ENUM_L1_SIZE_32_KB
}CacheL1Cfg;

typedef enum
{
	ENUM_L2_SIZE_0_KB=0,
	ENUM_L2_SIZE_32_KB,
	ENUM_L2_SIZE_64_KB,
	ENUM_L2_SIZE_128_KB,
	ENUM_L2_SIZE_256_KB,
	ENUM_L2_SIZE_512_KB
}CacheL2Cfg;


#define CACHE_L1D   1
#define CACHE_L1P   2
#define CACHE_L2    3


#ifdef __cplusplus
extern "C" {
#endif

void  CfgCache_L1_L2_Size(CacheL1Cfg L1PCfg, CacheL1Cfg L1DCfg, CacheL2Cfg L2Cfg);

void  Invalidate_Cache(unsigned char nType, void* pStatrt, unsigned int nbycnt, unsigned char blwatiFinsh);

void  Writeback_Cache(unsigned char nType, void* pStatrt, unsigned int nbycnt, unsigned char blwatiFinsh);

void  Invalid_Writeback_Cache(unsigned char nType, void* pStatrt, unsigned int nbycnt, unsigned char blwatiFinsh);

void  Invalidata_Cache_All(unsigned char nType, unsigned char blwatiFinsh);

#ifdef __cplusplus
}
#endif



#endif /* API_CACHE_H_ */
