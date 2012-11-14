#	Copyright 2012 Piers Lauder

#	This file is part of MHSnet.

#	MHSnet is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.

#	MHSnet is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.

#	You should have received a copy of the GNU General Public License
#	along with MHSnet.  If not, see <http://www.gnu.org/licenses/>.


/*
**	Routines for packet i/o.
*/

#define	SIGNALS

#include	"global.h"
#include	"debug.h"
#include	"driver.h"
#include	"packet.h"
#include	"chnstats.h"
#include	"channel.h"
#include	"dmnstats.h"
#include	"pktstats.h"
#include	"status.h"


#ifndef	ALIGN
#define		ALIGN		sizeof(char *)
#endif
#define		MASK		(ALIGN-1)


/*
**	Local data.
*/

static Uchar *	PBend;
static Uchar *	PBstart;
static Uchar *	PktBuffer;
static Uchar *	WriteBuffer;

#if	PROTO_STATS >= 1
static PktDir	Pkt_dir;
#endif	/* PROTO_STATS >= 1 */

static Uchar *	PassPackets _FA_((Uchar *, int));

/*
**	Global data.
*/

Uchar *		PktWriteData;
long		PktDatSize;
long		PktBufSize;

/*
**	Ensure packet data aligned on word boundary.
*/

void
InitPktData()
{
	Uchar *	buffer;

	/** Need enough room for largest control packet **/

	if ( (PktDatSize = OutDataSize) < HomeAdLength )
		PktDatSize = HomeAdLength;
	if ( PktDatSize < LinkAdLength )
		PktDatSize = LinkAdLength;
	if ( PktDatSize < SOMMDATASIZE )
		PktDatSize = SOMMDATASIZE;

	PktBufSize = MaxPktSize(PktDatSize);
	buffer = (Uchar *)Malloc(PktBufSize+ALIGN);

	for ( WriteBuffer = buffer ; ; WriteBuffer++ )
	{
		PktWriteData = A_PKTDATA(WriteBuffer);

		if ( ((PktWriteData - buffer) & MASK) == 0 )
			break;
	}

	PktBuffer = (Uchar *)Malloc(2*PktBufSize+ALIGN) + (WriteBuffer - buffer);
	PBstart = PktBuffer;
	PBend = PktBuffer;

	Trace3(1,
		"InitPktData(): PktWriteData=%#lx, PktReadData=%#lx",
		(PUlong)PktWriteData,
		(PUlong)A_PKTDATA(PktBuffer)
	);
}



/*
**	Adjust packet data size if necessary.
*/

void
AdjPktSize(throughput)
	register Ulong	throughput;	/* Correctly received RAW bytes/sec */
{					/* measured at remote */
	register Ulong	max_pkt_size;
	static Ulong	last_tp;
	static Ulong	last_change;
	static char	chngmsg[] = english("adjust packet size to %d (%lu b/s)");
	static char	thrpmsg[] = english("%s rate %screasing (%d -> %d b/s)");
	extern bool	Stats;
	extern int	WriteAheadSecs;

	if ( NoAdjust || NomSpeed == 0 )
		return;

	Trace5(
		1,
		"AdjPktSize(%lu) datasize %lu, speed %lu, last_change %lu",
		(PUlong)throughput, (PUlong)OutDataSize,
		(PUlong)NomSpeed, (PUlong)last_change
	);

	/*
	**	NomSpd	7/8	3/4	DataSiz	max
	**
	**	1440	1260	1080	1024	1424
	**	1440	1260	1080	256	1380
	**
	**	960	840	720	1024	949
	**	960	840	720	256	920
	**
	**	240	210	180	256	230
	**	240	210	180	64	204
	*/

	if ( throughput < ((NomSpeed * 3) / 4) )	/* < 155 b/s at 2400|64 */
	{
		if ( throughput > last_tp && last_tp > 0 )
		{
			if ( Verbose && Stats )
				Report5(thrpmsg, english("low"), english("in"), last_tp, throughput);
		}
		else
		if
		(
			(throughput < last_change || last_change == 0)
			&&
			OutDataSize > MINPKTDATASIZE
			&&
			OutDataSize > (throughput / 5)
		)
		{
			OutDataSize >>= 1;
			if ( OutDataSize == MAXCRCSIZE )
			{
				Status.st_flags &= ~CHDATACRC32;
				Status.st_flags |= CHDATACRC16;
			}
change:
			Status.st_pktdatasize = OutDataSize;
			if ( Stats )
				Report(chngmsg, OutDataSize, (PUlong)throughput);
			max_pkt_size = MaxPktSize(OutDataSize);
			InitDataCount = (WriteAheadSecs*NomSpeed+max_pkt_size-1)/max_pkt_size;
			last_change = throughput;
		}
	}
	else
	if ( throughput > ((NomSpeed * 7) / 8) )	/* > 181 b/s at 2400|64 */
	{
		if ( throughput < last_tp )
		{
			if ( Verbose && Stats )
				Report5(thrpmsg, english("high"), english("de"), last_tp, throughput);
		}
		else
		if
		(
			throughput > last_change
			&&
			OutDataSize < PktDatSize
			&&
			MaxPktSize(OutDataSize) < NomSpeed
		)
		{
			OutDataSize <<= 1;
			if ( OutDataSize == (MAXCRCSIZE*2) )
			{
				Status.st_flags &= ~CHDATACRC16;
				Status.st_flags |= CHDATACRC32;
			}
			goto change;
		}
	}

	last_tp = throughput;
}



/*
**	Return max packet size for OutDataSize.
*/

long
MaxPktSize(data_size)
	long		data_size;
{
	register int	pktsize;

	pktsize = data_size + PKTHDRSIZE + CRC_SIZE;

	if ( UseDataCRC )
		pktsize += (data_size > MAXCRCSIZE) ? CRC32_SIZE : CRC_SIZE;

	return pktsize;
}



/*
**	Read in packets via large buffer,
**	pass back complete packets via ``PktFuncsp''.
*/

void
PacketReader(funcp, fd, name)
	Funcp		funcp;
	int		fd;
	char *		name;
{
	register int	n;

	Trace4(3, "PacketReader fd %d name \"%s\" [kept=%d].", fd, name, PBend - PBstart);

#	if	PROTO_STATS >= 1
	Pkt_dir = (fd==VCfd) ? dir_vcin
			     : (fd==PinFd) ? dir_pipin
					   : dir_pipout;
#	endif	/* PROTO_STATS >= 1 */

	while ( (n = (*funcp)(fd, (char *)PBend, PktBufSize, name)) > 0 )
	{
		PBend += n;

		PBstart = PassPackets(PBstart, PBend - PBstart);

		if ( (n = PBend - PBstart) > 0 )
		{
			if ( PBend <= &PktBuffer[PktBufSize] )
				continue;

			if ( n > PktBufSize )
			{
				PKTSTATSINCC(PS_SKIPDATA, n-PktBufSize);
				n = PktBufSize;
				PBstart = PBend - n;
			}

			Trace2(3, "PacketReader move %d bytes", n);

			bcopy((char *)PBstart, (char *)&PktBuffer[0], n);
		}

		PBstart = &PktBuffer[0];
		PBend = &PktBuffer[n];
	}
}



/*
**	Find whole packets and pass them back to driver.
*/

static Uchar *
PassPackets(start, left)
	register Uchar *	start;
	register int		left;
{
	register int		size;
	register int		consumed;
	register vFuncp		funcp;
	register int		type;

	static bool		hcrc_ok;

	Trace4(3, "PassPackets(%#lx, %d)%s", (PUlong)start, left, hcrc_ok?" hcrc ok":EmptyString);

	while ( left >= (PKTHDRSIZE+CRC_SIZE) )
	{
		if ( !hcrc_ok && G_PKTHCRC(start) )
		{
			TracePkt(Pkt_dir, start, "BAD HEADER", false);
			PKTSTATSINC(PS_SKIPBYTE);
			left--, start++;
			hcrc_ok = false;
			continue;
		}

		hcrc_ok = true;
		consumed = PKTHDRSIZE + CRC_SIZE;

		if ( (size = G_PKTSIZE(start)) > 0 || G_PKTTYPE(start) == PKTDATATYPE )
		{
			if ( size > PktDatSize || size == 0 )
			{
				TracePkt(Pkt_dir, start, "BAD SIZE", true);
				PKTSTATSINC(PS_BADSIZE);
				left--, start++;
				hcrc_ok = false;
				continue;
			}

			consumed += size;

			if ( UseDataCRC )
			{
				consumed += (size>MAXCRCSIZE)?CRC32_SIZE:CRC_SIZE;
				if ( consumed > left )
					break;
				if ( G_PKTDCRC(start, size) )
				{
					TracePkt(Pkt_dir, start, "BAD DATA CRC", true);
					PKTSTATSINC(PS_BADDCRC);
					left--, start++;
					hcrc_ok = false;
					continue;
				}
			}
			else
			if ( consumed > left )
				break;
		}

		TracePkt(Pkt_dir, start, EmptyString, true);

		if ( (funcp = PktFuncsp[type = G_PKTTYPE(start)]) == NULLVFUNCP )
		{
			PKTSTATSINC(PS_BADTYPE);
			Fatal2("Bad PktFuncsp[%d]", type);
		}
		else
		{
			(*funcp)
			(
				G_PKTDIR(start),
				G_PKTCHAN(start),
				type,
				size,
				G_PKTADDR(start),
				A_PKTDATA(start)
			);
			PKTSTATSINC(PS_RPKTS);
		}

		if ( DataChannels != 0 )
			ActiveRawBytes += consumed * ACT_MULT;

		start += consumed;
		left -= consumed;
		hcrc_ok = false;
	}

	return start;
}



/*
**	Format and write a data packet to virtual circuit.
*/

bool
SendData(chnp)
	register Chan *	chnp;
{
	register int	n;
	register int	size = 0;
	register Ulong	addr = chnp->ch_msgaddr;

	if ( chnp->ch_flags & CH_EOF )
		Fatal2(english("SendData called at EOF on channel %d"), chnp->ch_number);

	do
	{
		if ( (n = ReadData(chnp, (char *)&PktWriteData[size], OutDataSize-size)) == 0 )
		{
			Trace3(1, "SendData channel %d read 0 at %lu", chnp->ch_number, (PUlong)(addr+size));
			BadMesg(chnp, "message truncated?");
			return false;
		}

		size += n;
	}
	while
		( size < OutDataSize && !(chnp->ch_flags & CH_EOF) );

	if ( !(chnp->ch_flags & CH_IGNBUF) )	/* !NAK */
	{
		chnp->ch_statsout(wch_data) += size;
		ActiveBytes += size * ACT_MULT;
		DMNSTATSINCC(DS_DATA, size);
	}

	S_PKTHDR(WriteBuffer, PKTDATATYPE, chnp->ch_number, Direction, size);
	S_PKTADDR(WriteBuffer, addr);
	S_PKTHCRC(WriteBuffer);

	if ( UseDataCRC )
	{
		if ( size > MAXCRCSIZE )
		{
			(void)crc32((char *)A_PKTDATA(WriteBuffer), size);
			size += PKTHDRSIZE+CRC_SIZE+CRC32_SIZE;
		}
		else
		{
			(void)crc((char *)A_PKTDATA(WriteBuffer), size);
			size += PKTHDRSIZE+CRC_SIZE+CRC_SIZE;
		}
	}
	else
		size += PKTHDRSIZE+CRC_SIZE;

	TracePkt(dir_vcout, WriteBuffer, EmptyString, true);
	PKTSTATSINC(PS_XPKTS);

	(void)(*WriteFuncp)(VCfd, (char *)WriteBuffer, size, VCname);

	return true;
}



/*
**	Format and write a packet to file (data already in PktWriteData.)
*/

int
WritePkt(chan, type, size, addr, fd, name)
	int		chan;
	int		type;
	register int	size;
	Ulong		addr;
	int		fd;
	char *		name;
{
	extern bool	WriterBlocked;

	if ( size > PktDatSize || size < 0 )
		Fatal3("Bad size (%d) passed to WritePkt on channel %d", size, chan);

	S_PKTHDR(WriteBuffer, type, chan, Direction, size);
	S_PKTADDR(WriteBuffer, addr);
	S_PKTHCRC(WriteBuffer);

	if ( size && UseDataCRC )
	{
		if ( size > MAXCRCSIZE )
		{
			(void)crc32((char *)A_PKTDATA(WriteBuffer), size);
			size += PKTHDRSIZE+CRC_SIZE+CRC32_SIZE;
		}
		else
		{
			(void)crc((char *)A_PKTDATA(WriteBuffer), size);
			size += PKTHDRSIZE+CRC_SIZE+CRC_SIZE;
		}
	}
	else
		size += PKTHDRSIZE+CRC_SIZE;

	TracePkt((fd==VCfd)?dir_vcout:(fd==PinFd)?dir_pipin:dir_pipout, WriteBuffer, EmptyString, true);
	PKTSTATSINC(PS_XPKTS);

	if ( fd != VCfd )	/* To writer */
	{
		WriterBlocked = true;
		(void)alarm(SLEEP_TIME+30);
	}

	size = (*WriteFuncp)(fd, (char *)WriteBuffer, size, name);

	if ( fd != VCfd )
	{
		WriterBlocked = false;
		if ( Idle_Timeout == 0 )
			(void)alarm((unsigned)0);

		if ( !(Status.st_flags & CHLINKSTART) )
		{
			if ( Time > LastSignalTime )
			{
				(void)kill(OtherPid, SIGWRITER);	/* Wakeup writer */
				LastSignalTime = Time;
			}
			else
				NeedSignal = true;			/* No more than 1/second */
		}
	}

	return size;
}
