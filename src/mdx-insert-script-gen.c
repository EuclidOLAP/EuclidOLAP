#include <stdio.h>

int main(int argc, char *argv[])
{
	char *region_mbrs[] = {
		"[Asia].[China]",
		"[Asia].[Japan]",
		"[Asia].[South Korea]",
		"[America].[U.S]",
		"[America].[Mexico]",
		"[America].[Chile]",
		"[Europe].[Greece]",
		"[Europe].[Italy]",
		"[Europe].[UK]",
	};

	int region_mbrs_count = 4;

	char *calendar_mbrs[] = {
		"[2019].[Q3].[M7]",
		"[2019].[Q3].[M8]",
		"[2019].[Q3].[M9]",
		"[2019].[Q4].[M10]",
		"[2019].[Q4].[M11]",
		"[2019].[Q4].[M12]",
		"[2020].[Q1].[M1]",
		"[2020].[Q1].[M2]",
		"[2020].[Q1].[M3]",
		"[2020].[Q2].[M4]",
		"[2020].[Q2].[M5]",
		"[2020].[Q2].[M6]",
		"[2020].[Q3].[M7]",
		"[2020].[Q3].[M8]",
		"[2020].[Q3].[M9]",
		"[2020].[Q4].[M10]",
		"[2020].[Q4].[M11]",
		"[2020].[Q4].[M12]",
		"[2021].[Q1].[M1]",
		"[2021].[Q1].[M2]",
		"[2021].[Q1].[M3]",
		"[2021].[Q2].[M4]",
		"[2021].[Q2].[M5]",
		"[2021].[Q2].[M6]",
	};

	int calendar_mbrs_count = 4;

	char *transport_mbrs[] = {
		"[railway]",
		"[highway]",
		"[aviation]",
		"[ocean freight]",
	};

	int transport_mbrs_count = 4;

	char *goods_mbrs[] = {
		"[foods].[nut]",
		"[foods].[wine]",
		"[foods].[beef]",
		"[electronic product].[mobile phone]",
		"[electronic product].[computer]",
		"[electronic product].[smart watch]",
		"[household appliances].[washing machine]",
		"[household appliances].[refrigerator]",
		"[household appliances].[television]",
	};

	int goods_mbrs_count = 4;

	int cell_count = goods_mbrs_count * transport_mbrs_count * region_mbrs_count * region_mbrs_count * calendar_mbrs_count * calendar_mbrs_count;
	printf("cell_count = %d\n", cell_count);
	int interval = cell_count / 1000;
	printf("interval = %d\n", interval);

	printf("./euclid-cli \" insert [logistics.test] \" \\\n");

	// dimensions Goods Goods Transport Transport Region [starting region] Region [ending region] Calendar [starting date] Calendar [completion date]
	int r_Goods, r_Transport, r_starting_region, r_ending_region, r_starting_date, r_completion_date;
	unsigned int i = 0;
	for (r_Goods = 0; r_Goods < goods_mbrs_count; r_Goods++)
	{
		for (r_Transport = 0; r_Transport < transport_mbrs_count; r_Transport++)
		{
			for (r_starting_region = 0; r_starting_region < region_mbrs_count; r_starting_region++)
			{
				for (r_ending_region = 0; r_ending_region < region_mbrs_count; r_ending_region++)
				{
					for (r_starting_date = 0; r_starting_date < calendar_mbrs_count; r_starting_date++)
					{
						for (r_completion_date = 0; r_completion_date < calendar_mbrs_count; r_completion_date++)
						{
							i++;
							// if (i % interval)
							// 	continue;

							char *goods_m = goods_mbrs[r_Goods];
							char *transport_m = transport_mbrs[r_Transport];
							char *starting_region_m = region_mbrs[r_starting_region];
							char *ending_region_m = region_mbrs[r_ending_region];
							char *starting_date_m = calendar_mbrs[r_starting_date];
							char *completion_date_m = calendar_mbrs[r_completion_date];
							printf("\" (Goods.%s, Transport.%s, [starting region].%s, [ending region].%s, [starting date].%s, [completion date].%s measures quantity %d.11 income %d.22 cost %d.33 ), \" \\\n",
								   goods_m, transport_m, starting_region_m, ending_region_m, starting_date_m, completion_date_m, i, i, i);
						}
					}
				}
			}
		}
	}

	return 0;
}