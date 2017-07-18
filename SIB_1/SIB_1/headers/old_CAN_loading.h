void ext_press_CAN(void)
{
	uint16_t diff1, diff2, diff3;
	uint16_t p_variation;
	uint8_t p1_dc = 0;
	uint8_t p2_dc = 0;
	uint8_t p3_dc = 0;					//indicates pressure sensor discontinuity - jumped to different depth
	uint8_t num_sensors = 0;								//indicates number of valid sensors incorporated in pressure output
	uint8_t errors = 0;									//number of discontinuities
	uint16_t ep1;
	uint16_t ep2;
	uint16_t ep3;								//current external pressure values
	uint32_t median;
	uint32_t min;
	uint32_t max;
	uint8_t prev_history;

	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		
		ep1 = ep1_raw[history];
		ep2 = ep2_raw[history];
		ep3 = ep3_raw[history];

		if (history >0)
		{
			prev_history = history - 1;
		}
		else
		{
			prev_history = 9;
		}

		//check pressure 1 continuity
		if(ep1 > ep1_raw[prev_history])
		{
			diff1 = ep1-ep1_raw[prev_history];
		}
		else
		{
			diff1 = ep1_raw[prev_history]-ep1;
		}

		//check pressure 2 continuity
		if(ep2 > ep2_raw[prev_history])
		{
			diff2 = ep2-ep2_raw[prev_history];
		}
		else
		{
			diff2 = ep2_raw[prev_history]-ep2;
		}

		//check pressure 3 continuity
		if(ep3 > ep3_raw[prev_history])
		{
			diff3 = ep3-ep3_raw[prev_history];
		}
		else
		{
			diff3 = ep3_raw[prev_history]-ep3;
		}

		if( (diff1 > 50) || (diff2 > 50) || (diff3 > 50))
		{
			crit_depth = 1;
			if(diff1>50)
			{
				p1_dc = 1;
				CI_Depth.data[1] |= p1_dc << 2;
			}
			else
			{
				p1_dc = 0;
				CI_Depth.data[1] &= 0b111111011;
			}
			if(diff2>50)
			{
				p2_dc = 1;
				CI_Depth.data[1] |= p1_dc << 2;
			}
			else
			{
				p2_dc = 0;
				CI_Depth.data[1] &= 0b111110111;
			}
			if(diff3>50)
			{
				p3_dc = 1;
			}
			else
			{
				p3_dc = 0;
				CI_Depth.data[1] &= 0b111101111;
			}
		}
		errors = p1_dc+p2_dc+p3_dc;

		switch (errors)
		{
			case 0:
			num_sensors = 3;
			if((ep1 == ep2) && (ep2 == ep3))
			{
				median = ep3;
			}
			else if ((ep1 == ep2) || (ep1 == ep3))
			{
				median = ep1;
			}
			else if (ep2 == ep3)
			{
				median = ep2;
			}
			else
			{
				median = getMedian_of3(ep1,ep2,ep3);
			}
			break;
			case 1:
			num_sensors = 2;
			if ((ep1 == ep2) || (ep1 == ep3))
			{
				median = ep1;
			}
			else if (ep2 == ep3)
			{
				median = ep2;
			}
			break;
			case 2:
			num_sensors=1;

			if(!p1_dc)
			{median = ep1;}

			else if(!p2_dc)
			{median = ep2;}

			else
			{median = ep3;}
			break;
			case 3:
			median = 0xFF;
			num_sensors = 0;
			break;
		}


		//Write critical Depth interrupt frame
		if(crit_depth)
		{
			CI_Depth.data[0] = median;
			CI_Depth.data[1] &= 0b00011100;	//clear lower byte except discontinuity flags
			CI_Depth.data[1] |= (median >> 8) | (num_sensors<<5);
		}

		//NEED TO MODIFY FRAME SO VARIATION IS 5 BITS
		max = getMax(ep1,ep2,ep3);
		min =  getMin(ep1,ep2,ep3);

		if(max != min)
		{
			p_variation = 100*(max-min)/min;
		}
		else
		{
			p_variation = 0;
		}
		
		Request_Response_SIB_Pressure.data[0] = 0;
		Request_Response_SIB_Pressure.data[1] = 0;
		Request_Response_SIB_Pressure.data[2] = 0;
		Request_Response_SIB_Pressure.data[3]  = (median<<2);
		Request_Response_SIB_Pressure.data[4]  = (median>>4) | (p_variation<<4);
		Request_Response_SIB_Pressure.data[5] = num_sensors;
		
		//DEBUG STRING
		sprintf(string_out, "median EP: %l", median);
		sendString(string_out);

		sprintf(string_out, "EP 1 DC: %l", p1_dc);
		sendString(string_out);

		sprintf(string_out, "EP 2 DC: %l", p2_dc);
		sendString(string_out);

		sprintf(string_out, "EP 3 DC: %l\n\r", p3_dc);
		sendString(string_out);
		//DEBUG STRING
		//sprintf(string_out, "EP: %d\n\r", Request_Response_SIB_Pressure );
		//sendString(string_out);
		//USART_CAN_TX(Request_Response_SIB_Pressure);
		//mcp2515_send_message(&Request_Response_SIB_Pressure);
		crit_error = 0;
	}
}

void int_temp_CAN(void)
{
	uint16_t diff1, diff2, diff3;
	uint16_t t_variation;
	uint8_t t1_dc = 0;
	uint8_t t2_dc = 0;
	uint8_t t3_dc = 0;					//indicates pressure sensor discontinuity - jumped to different depth
	uint8_t num_sensors = 0;								//indicates number of valid sensors incorporated in pressure output
	uint8_t errors = 0;									//number of discontinuities
	uint16_t t1;
	uint16_t t2;
	uint16_t t3;								//current external pressure values
	uint32_t median;
	uint32_t min;
	uint32_t max;
	uint8_t prev_history;
	uint8_t t_flag = 0;
	uint8_t dc_flag = 0;

	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		
		t1 = t1_raw[history];
		t2 = t2_raw[history];
		t3 = t2_raw[history];

		if (history >0)
		{
			prev_history = history - 1;
		}
		else
		{
			prev_history = 9;
		}

		//check pressure 1 continuity
		if(t1 > t1_raw[prev_history])
		{
			diff1 = t1-t1_raw[prev_history];
		}
		else
		{
			diff1 = t1_raw[prev_history]-t1;
		}

		//check pressure 2 continuity
		if(t2 > t2_raw[prev_history])
		{
			diff2 = t2-t2_raw[prev_history];
		}
		else
		{
			diff2 = t2_raw[prev_history]-t2;
		}

		//check pressure 3 continuity
		if(t3> t3_raw[prev_history])
		{
			diff3 = t3-t3_raw[prev_history];
		}
		else
		{
			diff3 = t3_raw[prev_history]-t3;
		}

		if((diff1 > 50) || (diff2 > 50) || (diff3 > 50))
		{
			if(diff1>50)
			{
				t1_dc = 1;
			}
			else
			{
				t1_dc = 0;
			}
			if(diff2>50)
			{
				t2_dc = 1;
			}
			else
			{
				t2_dc = 0;
			}
			if(diff3>50)
			{
				t3_dc = 1;
			}
			else
			{
				t3_dc = 0;
			}
		}
		else
		{
			t1_dc = 0;
			t2_dc= 0;
			t3_dc = 0;
		}
		errors = t1_dc+t2_dc+t3_dc;

		switch (errors)
		{
			case 0:
			num_sensors = 3;
			if((t1 == t2) && (t2 == t3))
			{
				median = t3;
			}
			else if ((t1 == t2) || (t1 == t3))
			{
				median = t1;
			}
			else if (t2 == t3)
			{
				median = t2;
			}
			else
			{
				median = getMedian_of3(t1,t2,t3);
			}
			break;
			case 1:
			num_sensors = 2;
			if ((t1 == t2) || (t1 == t3))
			{
				median = t1;
			}
			else if (t2 == t3)
			{
				median = t2;
			}
			break;
			case 2:
			num_sensors=1;

			if(!t1_dc)
			{median = t1;}

			else if(!t2_dc)
			{median = t2;}

			else
			{median = t3;}
			break;
			case 3:
			median = 0xFF;
			num_sensors = 0;
			crit_general = 1;
			dc_flag = 1;
			break;

		}

		if(median > T_CRIT)
		{
			crit_error = GENERAL;
			t_flag = 1;
			CI_SIB_General.data[0] |= 1;
			//write can bit T_CRIT FLAG
		}
		else
		{
			CI_SIB_General.data[0] &= 0b11111110;
			//clear t_crit flag
		}
		if(dc_flag)
		{
			CI_SIB_General.data[0] |= (1<<2);
			//set T_Discontinuity flag
		}
		else
		{
			CI_SIB_General.data[0] &= 0b11111011;
			//clear T_Discontinuity flag
		}

		//NEED TO MODIFY FRAME SO VARIATION IS 5 BITS
		max = getMax(t1,t2,t3);
		min =  getMin(t1,t2,t3);
		if(max != min)
		{
			t_variation = 100*(max-min)/min;
		}
		else
		{
			t_variation = 0;
		}
		
		Request_Response_SIB_Temp.data[0] = median;
		Request_Response_SIB_Temp.data[1] = median<<8;
		Request_Response_SIB_Temp.data[2] = (num_sensors<<4) | t_variation;

		
		//DEBUG STRING
		sprintf(string_out, "median T: %l", median);
		sendString(string_out);

		sprintf(string_out, "T1 DC: %l", t1_dc);
		sendString(string_out);

		sprintf(string_out, "T2 DC: %l", t2_dc);
		sendString(string_out);

		sprintf(string_out, "T3 DC: %d\n\r", t3_dc);
		sendString(string_out);
		//DEBUG STRING

		//USART_CAN_TX(Request_Response_SIB_Temp);
		//mcp2515_send_message(&Request_Response_SIB_Pressure);
	}

}

void int_press_CAN(void)
{
	uint16_t diff1, diff2, diff3;
	uint16_t p_variation;
	uint8_t p1_dc = 0;
	uint8_t p2_dc = 0;
	uint8_t p3_dc = 0;					//indicates pressure sensor discontinuity
	uint8_t num_sensors = 0;								//indicates number of valid sensors incorporated in pressure output
	uint8_t errors = 0;									//number of discontinuities
	uint32_t ip1;
	uint32_t ip2;
	uint32_t ip3;								//current internal pressure values
	uint32_t median;
	uint32_t min;
	uint32_t max;
	uint8_t prev_history;
	uint8_t dc_flag = 0;						//raised if all 3 experience discontinuity

	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		
		ip1 = ip1_raw[history];
		ip2 = ip2_raw[history];
		ip3 = ip3_raw[history];

		if (history >0)
		{
			prev_history = history - 1;
		}
		else
		{
			prev_history = 9;
		}

		//check pressure 1 continuity
		if(ip1 > ip1_raw[prev_history])
		{
			diff1 = ip1-ip1_raw[prev_history];
		}
		else
		{
			diff1 = ip1_raw[prev_history]-ip1;
		}

		//check pressure 2 continuity
		if(ip2 > ip2_raw[prev_history])
		{
			diff2 = ip2-ip2_raw[prev_history];
		}
		else
		{
			diff2 = ip2_raw[prev_history]-ip2;
		}

		//check pressure 3 continuity
		if(ip3 > ip3_raw[prev_history])
		{
			diff3 = ip3-ip3_raw[prev_history];
		}
		else
		{
			diff3 = ip3_raw[prev_history]-ip3;
		}

		if((diff1 > 50) || (diff2 > 50) || (diff3 > 50))
		{
			if(diff1>50)
			{
				p1_dc = 1;
			}
			else
			{
				p1_dc = 0;
			}
			if(diff2>50)
			{
				p2_dc = 1;
			}
			else
			{
				p2_dc = 0;
			}
			if(diff3>50)
			{
				p3_dc = 1;
			}
			else
			{
				p3_dc = 0;
			}
		}
		else
		{
			p1_dc = 0;
			p2_dc = 0;
			p3_dc = 0;
		}
		errors = p1_dc+p2_dc+p3_dc;

		switch (errors)
		{
			case 0:
			num_sensors = 3;
			max = getMax(ip1,ip2,ip3);
			min =  getMin(ip1,ip2,ip3);
			if((ip1 == ip2) && (ip2 == ip3))
			{
				median = ip3;
			}
			else if ((ip1 == ip2) || (ip1 == ip3))
			{
				median = ip1;
			}
			else if (ip2 == ip3)
			{
				median = ip2;
			}
			else
			{
				median = getMedian_of3(ip1,ip2,ip3);
			}
			break;
			case 1:
			num_sensors = 2;
			if ((ip1 == ip2) || (ip1 == ip3))
			{
				median = ip1;
			}
			else if (ip2 == ip3)
			{
				median = ip2;
			}
			else if  (p1_dc)
			{
				median = ip2;
			}
			else
			{
				median = ip1;
			}
			break;
			case 2:
			num_sensors=1;

			if(!p1_dc)
			{median = ip1;}

			else if(!p2_dc)
			{median = ip2;}

			else
			{median = ip3;}
			break;
			case 3:
			median = 0xFF;
			num_sensors = 0;
			crit_general =1;
			dc_flag = 1;
			break;
		}

		//Write critical Depth interrupt frame
		if(dc_flag)
		{
			CI_SIB_General.data[0] = dc_flag << 1;
		}
		else
		{
			//clear dc_flag
			CI_SIB_General.data[0] &= 0b11111101;
		}

		//NEED TO MODIFY FRAME SO VARIATION IS 5 BITS


		if(max != min)
		{
			p_variation = 100*(max-min)/min;
		}
		else
		{
			p_variation = 0;
		}
		
		Request_Response_SIB_Pressure.data[0] = median;
		Request_Response_SIB_Pressure.data[1] = median<<8;
		Request_Response_SIB_Pressure.data[2] = (median <<16) | (p_variation <<4);
		Request_Response_SIB_Pressure.data[3]  |= num_sensors;
		
		//DEBUG STRING
		sprintf(string_out, "median IP: %l", median);
		sendString(string_out);

		sprintf(string_out, "IP 1 DC: %l", p1_dc);
		sendString(string_out);

		sprintf(string_out, "IP 2 DC: %l", p2_dc);
		sendString(string_out);

		sprintf(string_out, "IP 3 DC: %d\n\r", p3_dc);
		sendString(string_out);
		//DEBUG STRING

		//USART_CAN_TX(Request_Response_SIB_Pressure);
		//mcp2515_send_message(&Request_Response_SIB_Pressure);
	}
}