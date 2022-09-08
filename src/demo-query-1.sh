#!/bin/bash

./euclid-cli \
"select " \
"{ " \
"([Calendar].[2020].[Q1]), " \
"([Calendar].[2020].[Q2]), " \
"([Calendar].[2020].[Q3]), " \
"([Calendar].[2020].[Q4]) " \
"} on 0, " \
"{ " \
"([Payment Method].[Credit Card], measure.[sales amount]), " \
"([Payment Method].[Debit Card], measure.[sales amount]), " \
"([Payment Method].[Account Balance], measure.[sales amount]) " \
"} on 1 " \
"from [Online Store]"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with " \
"member Calendar.[First half 2020]  as ([Calendar].[2020].[Q1]) + ([Calendar].[2020].[Q2]) " \
"member Calendar.[Second half 2020] as ([Calendar].[2020].[Q3]) + ([Calendar].[2020].[Q4]) " \
"select " \
"{ " \
"([Calendar].[2020].[Q1]), " \
"([Calendar].[2020].[Q2]), " \
"([Calendar].[2020].[Q3]), " \
"([Calendar].[2020].[Q4]), " \
"(Calendar.[First half 2020]), " \
"(Calendar.[Second half 2020]) " \
"} on 0, " \
"{ " \
"([Payment Method].[Credit Card]), " \
"([Payment Method].[Debit Card]), " \
"([Payment Method].[Account Balance]) " \
"} on 1 " \
"from [Online Store]"

echo "........................................................................................... done"
sleep 1


./euclid-cli \
"with member [Payment Method].PROPORTION as ([Payment Method].[ALL].[Credit Card]) / ([Payment Method].[ALL])" \
"select " \
"{ " \
"([Calendar].[2020].[Q1]), " \
"([Calendar].[2020].[Q2]), " \
"([Calendar].[2020].[Q3]), " \
"([Calendar].[2020].[Q4]) " \
"} on 0, " \
"{ ([Payment Method].[ALL].[Credit Card]), ([Payment Method].[ALL]), ([Payment Method].PROPORTION) } on 1 " \
"from [Online Store] where (measure.[sales amount])"

echo "........................................................................................... done"
sleep 1


./euclid-cli \
"select " \
"children([Goods].[Kitchen & Dining]) on 0, " \
"children([Calendar].[ALL].[2021].[Q4]) on 1" \
"from [Online Store] where (measure.[sales amount])"

echo "........................................................................................... done"
sleep 1


./euclid-cli \
"select " \
"members([Goods], NOT_LEAFS) on 10, " \
"members(Calendar, LEAFS) on 100 " \
"from [Online Store]"

echo "........................................................................................... done"
sleep 1


./euclid-cli \
"select " \
"{ (measure.[sales amount]), (measure.[sales quantity]) } on 10, " \
"crossjoin(members([Store Type], LEAFS), members([Payment Method], ALL))  on 100 " \
"from [Online Store]"

echo "........................................................................................... done"
sleep 1


./euclid-cli \
"select " \
"{ (measure.[sales amount]), (measure.[sales quantity]) } on 10, " \
"crossjoin(members([Store Type], LEAFS), members([Payment Method], ALL), children([Calendar].[ALL].[2021].[Q4]))  on 100 " \
"from [Online Store]"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select " \
"{ (measure.[sales quantity]), (measure.[sales amount]) } on 10, " \
"crossjoin(members([Store Type], LEAFS), members([Payment Method], ALL), children([Calendar].[ALL].[2021].[Q4]), children(Goods.[ALL]))  on 100 " \
"from [Online Store]"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select " \
"members(measure) on 0, " \
"crossjoin(children([Store Type].[ALL]), members([Payment Method])) on 1 " \
"from [Online Store]"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select " \
"{ " \
"  (  [starting region].[ALL].[Asia].[China],  Transport.[ALL].railway  ), " \
"  (  [starting region].[ALL].[Asia].[China],  Transport.[ALL].railway, [measure].[quantity] ), " \
"  ( [starting region].[ALL].[Asia].[China], Transport.[ALL].highway, [measure].[quantity] ), " \
"  ( Goods.[ALL].[foods].[nut], Transport.[ALL].[railway], [starting region].[ALL].[Asia].[China] )  " \
"} on 0, " \
"{ " \
"  ( [starting date].[ALL].[2019].[Q4].[M10], Goods.[ALL].[foods].[nut], [measure].[income] ), " \
"  ( [starting date].[ALL].[2019].[Q4].[M10], Goods.[ALL].[foods].[nut] ), " \
"  (  [starting date].[ALL].[2019].[Q3].[M9],   Goods.[ALL].[foods].[wine]  ), " \
"  ( [ending region].[ALL].[Asia].[China], [starting date].[ALL].[2019].[Q3].[M7], [completion date].[ALL].[2019].[Q3].[M7] )  " \
"} on 1 " \
"from [logistics.test] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select	 " \
"{	 " \
"  ([Goods].[foods],[Transport].[railway],[starting region].[Europe].[UK]),	 " \
"  ([Transport].[highway],[starting region].[Asia].[Japan])  	 " \
"} on 0,	 " \
"{	 " \
"  ([starting region].[America].[Chile],[ending region].[Asia].[China]),	 " \
"  ([ending region].[Asia].[South Korea]),	 " \
"  ([starting date].[2019].[Q3].[M8],[completion date].[2020].[Q2],[Goods].[foods])	 " \
"} on 1,	 " \
"{	 " \
"  ([completion date].[2020].[Q4],[Goods].[foods],[Transport].[ocean freight]),	 " \
"  ([Goods].[foods],[Transport].[railway]),	 " \
"  ([Transport].[highway]),	 " \
"  ([starting region].[Europe].[Greece])	 " \
"} on 2	 " \
"from [logistics.test]"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select	 " \
"{ " \
"    ( " \
"        Goods.[foods].[nut], " \
"        Transport.[highway], " \
"        [starting region].[Europe].[UK], " \
"        [ending region].[Asia].[South Korea], " \
"        [starting date].[2019].[Q3].[M8], " \
"        [completion date].[2020].[Q4] " \
"    ), " \
"    ( " \
"        [ending region].[Asia].[South Korea], " \
"        [starting date].[2019].[Q3].[M8], " \
"        [completion date].[2020].[Q4], " \
"        Goods.[foods].[nut], " \
"        Transport.[highway], " \
"        [starting region].[Europe].[UK] " \
"    ) " \
"} on 0 " \
"from [logistics.test] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select " \
"{ " \
"  ( Goods.[foods].[beef], Transport.[aviation], [starting region].[Asia].[Japan], [ending region].[America].[U.S], [starting date].[2019].[Q3].[M7] ), " \
"  ( Goods.[foods].[beef], Transport.[aviation], [starting region].[Asia].[Japan], [ending region].[America].[U.S], [starting date].[2019].[Q3].[M7] ), " \
"  ( Goods.[foods].[beef], Transport.[aviation], [starting region].[Asia].[Japan], [ending region].[America].[U.S], [starting date].[2019].[Q3].[M7] ), " \
"  ( [starting region].[Europe].[UK], [ending region].[Europe].[Italy], [measure].[quantity] ), " \
"  ( [starting region].[Europe].[UK], [ending region].[Europe].[Italy], [measure].[income] ), " \
"  ( [starting region].[Europe].[UK], [ending region].[Europe].[Italy], [measure].[cost] ), " \
"  ( [starting region].[America].[Chile], [ending region].[Europe].[UK], [starting date].[2020].[Q4].[M11], [completion date].[2020].[Q4].[M12], [measure].[quantity] ), " \
"  ( Goods.[household appliances].[television], Transport.[ocean freight], [starting region].[America].[Chile], [ending region].[Europe].[UK] ), " \
"  ( Goods.[household appliances].[television], Transport.[ocean freight], [starting region].[America].[Chile], [ending region].[Europe].[UK], [starting date].[2020].[Q4].[M11], [completion date].[2020].[Q4].[M12] ) " \
"} on 0, " \
"{ " \
"  ( [completion date].[2020].[Q1].[M1], [measure].quantity ), " \
"  ( [completion date].[2020].[Q1].[M1], [measure].income ), " \
"  ( [completion date].[2020].[Q1].[M1], [measure].cost ), " \
"  ( Goods.[household appliances].[television], Transport.[ocean freight], [starting date].[2021].[Q2].[M5], [completion date].[2020].[Q1].[M3] ), " \
"  ( Goods.[household appliances].[television], Transport.[ocean freight], [starting date].[2021].[Q2].[M5], [completion date].[2020].[Q1].[M3] ), " \
"  ( [starting date].[2021].[Q2].[M5], [completion date].[2020].[Q1].[M3], Goods.[household appliances].[television], Transport.[ocean freight] ), " \
"  ( Goods.[household appliances].[television], Transport.[ocean freight] ), " \
"  ( [ending region].[Europe].[UK], [starting date].[2020].[Q4].[M11], [completion date].[2020].[Q4].[M12], [measure].[income] ), " \
"  ( Goods.[household appliances].[television], Transport.[ocean freight], [starting region].[America].[Chile], [ending region].[Europe].[UK], [starting date].[2020].[Q4].[M11], [completion date].[2020].[Q4].[M12], [measure].[cost] ) " \
"} on 1 " \
"from [logistics.test] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select " \
"{ " \
"  ([ending region].[Europe].[Italy], [completion date].[2020].[Q1].[M3], measure.cost ) " \
"} on 0, " \
"{ " \
"  (Goods.[household appliances].[television], Transport.[ocean freight], [starting region].[Europe].[UK] ) " \
"} on 1, " \
"{ " \
"  ( [starting date].[2021].[Q2].[M5] ) " \
"} on 2 " \
"from [logistics.test] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select " \
"{ " \
"  ([ending region].[Europe].[Italy], [completion date].[2020].[Q1].[M3], measure.cost ) " \
"} on 0, " \
"{ " \
"  (Goods.[household appliances].[television], Transport.[ocean freight], [starting region].[Europe].[UK] ) " \
"} on 1 " \
"from [logistics.test] where ( [starting date].[2021].[Q2].[M5] )"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select " \
"{ " \
"  ( measure.cost, [ending region].[Asia].[China] ), " \
"  ( measure.cost, [ending region].[Asia].[Japan] ), " \
"  ( measure.cost, [ending region].[Asia].[South Korea] ), " \
"  ( measure.cost, [ending region].[America].[U.S] ), " \
"  ( measure.cost, [ending region].[America].[Mexico] ), " \
"  ( measure.cost, [ending region].[America].[Chile] ), " \
"  ( measure.cost, [ending region].[Europe].[Greece] ), " \
"  ( measure.cost, [ending region].[Europe].[Italy] ), " \
"  ( measure.cost, [ending region].[Europe].[UK] ), " \
"  ( measure.cost, [ending region].[Asia] ), " \
"  ( measure.cost, [ending region].[America] ), " \
"  ( measure.cost, [ending region].[Europe] ) " \
"} on 0 " \
"from [logistics.test] where ( Goods.[foods].[nut], Transport.[railway], [starting region].[Asia].[China], [starting date].[2019].[Q3].[M7], [completion date].[2019].[Q3].[M7] )"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with member [measure].[MM001] as (measure.cost) + (measure.cost) * (measure.cost) - (measure.cost) " \
"select " \
"{ " \
"  ( measure.cost, [ending region].[Asia].[China] ), " \
"  ( measure.cost, [ending region].[Asia].[Japan] ), " \
"  ( measure.cost, [ending region].[Asia].[South Korea] ), " \
"  ( measure.cost, [ending region].[America].[U.S] ), " \
"  ( measure.cost, [ending region].[America].[Mexico] ), " \
"  ( measure.cost, [ending region].[America].[Chile] ), " \
"  ( measure.cost, [ending region].[Europe].[Greece] ), " \
"  ( measure.cost, [ending region].[Europe].[Italy] ), " \
"  ( measure.cost, [ending region].[Europe].[UK] ), " \
"  ( measure.cost, [ending region].[Asia] ), " \
"  ( measure.cost, [ending region].[America] ), " \
"  ( measure.cost, [ending region].[Europe] ), " \
"  ( measure.MM001, [ending region].[Europe] ), " \
"  ( measure.MM001, [ending region].[Asia] ) " \
"} on 0 " \
"from [logistics.test] where ( Goods.[foods].[nut], Transport.[railway], [starting region].[Asia].[China], [starting date].[2019].[Q3].[M7], [completion date].[2019].[Q3].[M7] )"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with " \
"member [ending region].CCCCCC as ([ending region].[Asia].[China]) * ([ending region].[America].[U.S]) / ([ending region].[Europe].[Greece]) + ([ending region].[Europe].[Italy]) " \
"member [ending region].AAAAAA as ([ending region].[Asia].[China]) * 1000000000 + ([ending region].[America].[U.S]) " \
"member [ending region].X as (([ending region].[Europe].[Greece]) * ((([ending region].[Asia].[China]) * ([ending region].[America].[U.S]) / ([ending region].[Europe].[Greece]) + ([ending region].[Europe].[Italy])) + 1000) - ([ending region].[Europe].[Italy])) / 0.5 " \
"member [ending region].XX as ((((((((((((((((((([ending region].[Asia]))))))))))))))))))) + ((([ending region].[Asia].[China]))) * 10000" \
"select " \
"{ " \
"  ( measure.cost, [ending region].[Asia].[China] ), " \
"  ( measure.cost, [ending region].[America].[U.S] ), " \
"  ( measure.cost, [ending region].[Europe].[Greece] ), " \
"  ( measure.cost, [ending region].[Europe].[Italy] ), " \
"  ( measure.cost, [ending region].CCCCCC ), " \
"  ( measure.cost, [ending region].AAAAAA ), " \
"  ( measure.cost, [ending region].X ), " \
"  ( measure.cost, [ending region].XX ) " \
"} on 0 " \
"from [logistics.test] where ( Goods.[foods].[nut], Transport.[railway], [starting region].[Asia].[China], [starting date].[2019].[Q3].[M7], [completion date].[2019].[Q3].[M7] )"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with " \
"set SSSSSS as children( [ending region].Asia ) " \
"select SSSSSS on 999 " \
"from [logistics.test] where ( Goods.[foods].[nut], Transport.[railway], [starting region].[Asia].[China], [starting date].[2019].[Q3].[M7], [completion date].[2019].[Q3].[M7] )"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with set QQQ as XXX " \
"set XXX as children( [Goods].[electronic product] ) " \
"select " \
"children([starting date].[2021]) on 0, " \
"QQQ on 111 " \
"from [logistics.test] " \
"where ( " \
"[completion date].[2020].[Q1].[M3], " \
"Transport.[railway], " \
"[starting region].[Europe].[UK], " \
"[ending region].[Europe].[Italy], " \
"measure.cost " \
")"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with set QQQ as XXX " \
"set XXX as children( parent([Goods].[electronic product].computer) ) " \
"select " \
"children( parent( parent( [starting date].[2021].Q1.M3 ) ) ) on 0, " \
"QQQ on 111 " \
"from [logistics.test] " \
"where ( " \
"[completion date].[2020].[Q1].[M3], " \
"Transport.[railway], " \
"[starting region].[Europe].[UK], " \
"[ending region].[Europe].[Italy], " \
"measure.income " \
")"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with " \
"member [measure].[FFFFFF] as sum( {(measure.income), (measure.cost)}, ([Goods].[electronic product].computer) ) " \
"select " \
"{ (measure.cost), (measure.income), ([measure].[FFFFFF]) } on 0, " \
"children( [starting date].[2021].Q1 ) on 1 " \
"from [logistics.test] " \
"where ( " \
"[Goods].[electronic product].[mobile phone], " \
"[completion date].[2020].[Q1].[M3], " \
"Transport.[railway], " \
"[starting region].[Europe].[UK], " \
"[ending region].[Europe].[Italy] " \
")"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with " \
"member [measure].[FFFFFF] as sum( {(measure.income), (measure.cost)}, ([Goods].[electronic product].computer) ) " \
"member [starting date].SSSSSS as sum( children( [starting date].[2021] ), ([Goods].[electronic product].computer) ) " \
"select " \
"{ (measure.cost), (measure.income), ([measure].[FFFFFF]) } on 0, " \
"{ ([starting date].[2021].Q1.M1), ([starting date].[2021].Q1.M2), ([starting date].[2021].Q1.M3), ([starting date].SSSSSS) } on 1 " \
"from [logistics.test] " \
"where ( " \
"[Goods].[electronic product].[mobile phone], " \
"[completion date].[2020].[Q1].[M3], " \
"Transport.[railway], " \
"[starting region].[Europe].[UK], " \
"[ending region].[Europe].[Italy] " \
")"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with " \
"member [starting date].SSSSSS as sum( { ([starting date].[2021].Q1), ([starting date].[2021].Q2), ([starting date].[2021].Q3), ([starting date].[2021].Q4) } ) " \
"member Goods.GGGGGG as sum( { ([Goods].[electronic product].[mobile phone]), ([Goods].[electronic product].computer), ([Goods].[electronic product].[smart watch]) } ) " \
"select " \
"{ ([starting date].[2021].Q1), ([starting date].[2021].Q2), ([starting date].[2021].Q3), ([starting date].[2021].Q4), ([starting date].SSSSSS) } on 0, " \
"{ ([Goods].[electronic product].[mobile phone]), ([Goods].[electronic product].computer), ([Goods].[electronic product].[smart watch]), (Goods.GGGGGG) } on 1 " \
"from [logistics.test] " \
"where ( Transport.railway, [starting region].[Europe].[UK], [ending region].[Europe].[Italy], [completion date].[2020].[Q1].[M3], measure.cost )"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select " \
"{ ([starting date].[2021].Q1), ([starting date].[2021].Q2) } on 999, " \
"{ (measure.income), (measure.cost) } on 100 " \
"from [logistics.test] " \
"where ( [Goods].[electronic product].[computer] , Transport.railway,  [starting region].[Europe].[UK],  [ending region].[Europe].[Italy],  [completion date].[2020].[Q1].[M3] )"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select " \
"children( [starting date].[2021].Q1 ) on 999, " \
"{ (measure.income), (measure.cost) } on 100 " \
"from [logistics.test] " \
"where ( [Goods].[electronic product].[computer] , Transport.railway,  [starting region].[Europe].[UK],  [ending region].[Europe].[Italy],  [completion date].[2020].[Q1].[M3] )"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select " \
" members(Goods) on 0, " \
" crossJoin(members(measure), members([ending region])) on 1 " \
"from [logistics.test] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with member [starting date].PPPPPP as ([starting date].[2021].Q1.M2) + ([starting date].[2021].Q1.M3) " \
"  member [measure].MMMMMM as (measure.income) * (measure.cost) " \
"select " \
"{ (measure.income), (measure.cost), ([measure].MMMMMM) } on 100, " \
"{ ([starting date].[2021].Q1.M1), ([starting date].[2021].Q1.M2), ([starting date].[2021].Q1.M3), ([starting date].PPPPPP) } on 200 " \
"from [logistics.test] " \
"where ( [Goods].[electronic product].[computer] , Transport.railway,  [starting region].[Europe].[UK],  [ending region].[Europe].[Italy],  [completion date].[2020].[Q1].[M3] )"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with member [starting date].PPPPPP as ([starting date].[2021].Q1.M2) + ([starting date].[2021].Q1.M3) " \
"  member [measure].MMMMMM as (measure.income) * (measure.cost) " \
"select " \
"{ (measure.income), (measure.cost), ([measure].MMMMMM) } on 1000, " \
"{ ([starting date].[2021].Q1.M1), ([starting date].[2021].Q1.M2), ([starting date].[2021].Q1.M3), ([starting date].PPPPPP) } on 200 " \
"from [logistics.test] " \
"where ( [Goods].[electronic product].[computer] , Transport.railway,  [starting region].[Europe].[UK]," \
"[ending region].[Europe].[Italy],  [completion date].[2020].[Q1].[M3] )"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with  " \
" member measure.SSSSSS as sum({ (measure.XXX), (measure.YYY), (measure.ZZZ) }) " \
" member measure.XXX as ([measure].[sales amount]) * 10 " \
"      member measure.YYY as ((measure.XXX) + 9900000000) " \
"      member measure.ZZZ as (measure.YYY) / 8 - 222.22 " \
" select " \
" { ([measure].[sales amount]), (measure.XXX), (measure.YYY), (measure.ZZZ), (measure.SSSSSS) } on 0, " \
" members(Calendar) on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with  " \
" member measure.SSSSSS as sum({ (measure.XXX), (measure.YYY), (measure.ZZZ) }) " \
" member measure.XXX as ([measure].[sales amount]) * 10 " \
"      member measure.YYY as ((measure.XXX) + 9900000000) " \
"      member measure.ZZZ as (measure.YYY) / 8 - 222.22 " \
" member measure.QQQ as sum({([measure].[sales amount]), ([measure].[cash back])}) " \
" member Calendar.VVV as sum({ ([Calendar].[ALL].[2020]), ([Calendar].[ALL].[2019]) })" \
" select " \
" { ([measure].[sales amount]), ([measure].[cash back]), (measure.XXX), (measure.YYY), (measure.ZZZ), (measure.SSSSSS) } on 0, " \
" { ([Calendar].[ALL].[2019]),  ([Calendar].[ALL].[2020]), (Calendar.VVV) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with  " \
" member Calendar.SSSSSS as count(members(Calendar)) " \
" member Calendar.EEE as count(members(Calendar), EXCLUDEEMPTY) " \
" member Calendar.III as count(members(Calendar), INCLUDEEMPTY) " \
" select " \
" { ([measure].[sales amount]), ([measure].[cash back]) } on 0, " \
" { ([Calendar].[ALL].[2019]),  ([Calendar].[ALL].[2020]), (Calendar.SSSSSS), (Calendar.EEE), (Calendar.III) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with  " \
" member Calendar.SSSSSS as count(members(Calendar)) " \
" member Calendar.EEE as count(members(Calendar), EXCLUDEEMPTY) " \
" member Calendar.III as count(members(Calendar), INCLUDEEMPTY) " \
" select " \
" { ([measure].[sales amount]), ([measure].[cash back]) } on 0, " \
" members(Calendar) on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli "select {(measure.[sales amount])} on 0, {(measure.[sales amount])} on 1 from [Online Store]"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" { ([measure].[sales amount]) } on 0, " \
" filter({ ([Calendar].[ALL].[2020].[Q1]), ([Calendar].[ALL].[2020]) }, ([measure].[sales amount]) > 29.999 ) on 1 " \
" from [Online Store] where " \
"([Store Type].[ALL].[Platform Self-operated Store],[Payment Method].[ALL].[Credit Card],[Goods].[ALL].[Kitchen & Dining].[Bento Boxes])"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" members(measure) on 1, " \
" filter(members(Calendar), ((((([measure].[sales amount]) < 240)))) ANd ((((([measure].[sales amount]) >= 30)))) ) on 99999999 " \
" from [Online Store] " \
" where " \
" ([Store Type].[ALL].[Platform Self-operated Store],[Payment Method].[ALL].[Credit Card],[Goods].[ALL].[Kitchen & Dining].[Bento Boxes]) "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" { ([Store Type].[ALL]) } on 0, " \
" filter(members(Calendar), ([Store Type].[ALL]) > 17279.999999999) on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" { ([Store Type].[ALL]) } on 0, " \
" filter(members(Calendar), (([Store Type].[ALL]) <= 2000) oR ([Store Type].[ALL]) > 30000) on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select " \
"{ " \
"  (  [starting region].[ALL].[Asia].[China],  Transport.[ALL].railway  ) " \
"} on 0, " \
"filter({ " \
"  ( [starting date].[ALL].[2019].[Q4].[M10], Goods.[ALL].[foods].[nut], [measure].[income] ), " \
"  ( [starting date].[ALL].[2019].[Q4].[M10], Goods.[ALL].[foods].[nut] ), " \
"  (  [starting date].[ALL].[2019].[Q3].[M9],   Goods.[ALL].[foods].[wine]  ), " \
"  ( [ending region].[ALL].[Asia].[China], [starting date].[ALL].[2019].[Q3].[M7], [completion date].[ALL].[2019].[Q3].[M7] )  " \
"}, ((Transport.[ALL].railway) > 0) OR (1 < 2)) on 1 " \
"from [logistics.test] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with member [Payment Method].PROPORTION as ([Payment Method].[ALL].[Credit Card]) / ([Payment Method].[ALL])" \
"select " \
"{ " \
"([Calendar].[2020].[Q4]) " \
"} on 10, " \
"filter( { ([Payment Method].[ALL].[Credit Card]), ([Payment Method].[ALL]), ([Payment Method].PROPORTION) }, (measure.[sales amount]) <> 4320 ) on 1 " \
"from [Online Store] where (measure.[sales amount])"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with  " \
" member measure.SSSSSS as sum({ (measure.XXX), (measure.YYY), (measure.ZZZ) }) " \
" member measure.XXX as ([measure].[sales amount]) * 10 " \
"      member measure.YYY as ((measure.XXX) + 111) " \
"      member measure.ZZZ as (measure.YYY) / 8 - 222.22 " \
" member measure.QQQ as sum({([measure].[sales amount]), ([measure].[cash back])}) " \
" member Calendar.VVV as sum({ ([Calendar].[ALL].[2020]), ([Calendar].[ALL].[2019]) })" \
" select " \
" { (measure.SSSSSS), ([measure].[sales amount]), ([measure].[cash back]), (measure.XXX), (measure.YYY), (measure.ZZZ) } on 0, " \
" filter(members(Calendar), (((measure.SSSSSS) <= 30000) or ((measure.SSSSSS) >= 40000)) and (((measure.SSSSSS) <= 360000) or ((measure.SSSSSS) >= 370000))) on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with set QQQ as XXX " \
"set XXX as children( parent([Goods].[electronic product].computer) ) " \
"select " \
"children( parent( parent( [starting date].[2021].Q1.M3 ) ) ) on 0, " \
"filter(QQQ, (parent( [starting date].[2021].Q1.M3 )) <> 9003) on 111 " \
"from [logistics.test] " \
"where ( " \
"[completion date].[2020].[Q1].[M3], " \
"Transport.[railway], " \
"[starting region].[Europe].[UK], " \
"[ending region].[Europe].[Italy], " \
"measure.income " \
")"

echo "........................................................................................... done"
sleep 1

# ./euclid-cli \
# " select " \
# " crossjoin(members([Store Type]), members([Payment Method]), members([Goods]), members([Calendar])) on 0, " \
# " {(measure.[sales amount]), (measure.[sales amount]), (measure.[sales amount]) , (measure.[sales amount]), (measure.[sales amount]) , (measure.[sales amount]), (measure.[sales amount]) , (measure.[sales amount]), (measure.[sales amount]) , (measure.[sales amount]), (measure.[sales amount]) , (measure.[sales amount]), (measure.[sales amount])  } on 1 " \
# " from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"WITH set QQQ as XXX " \
"set XXX as children( parent([Goods].[electronic product].computer) ) " \
"SELECT " \
"chILDren( parent( parent( [starting date].[2021].Q1.M3 ) ) ) On 0, " \
"filter(QQQ, (parent( [starting date].[2021].Q1.M3 )) <> 9003) On 111 " \
"frOM [logistics.test] " \
"whERE ( " \
"[completion date].[2020].[Q1].[M3], " \
"Transport.[railway], " \
"[starting region].[Europe].[UK], " \
"[ending region].[Europe].[Italy], " \
"measure.income " \
")"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" { ([Store Type].[ALL]) } on 0, " \
" { (parent(Calendar.[ALL].[2020]), parent([Payment Method].[ALL])) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with MEmber measure.XXX as ([measure].[sales amount]) / (paREnt(CurrentMember(Calendar)), [measure].[sales amount]) " \
" select " \
" { ([measure].[sales amount]), (measure.XXX) } ON 0, " \
" FIlter(members(Calendar), ([measure].[sales amount]) > 4000) on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with member [starting date].PPPPPP as ([starting date].[2021].Q1.M2) + ([starting date].[2021].Q1.M3) " \
"  member [measure].MMMMMM as (prevMember(currentMember([starting date])), measure.cost) " \
"select " \
"{ (measure.cost), ([measure].MMMMMM) } on 1, " \
"filter(members([starting date]), (measure.cost) > 1) on 200 " \
"from [logistics.test] " \
" where ( [Goods].[ALL].[electronic product].[computer] , Transport.[ALL].railway,  [starting region].[ALL].[Europe].[UK],  [ending region].[ALL].[Europe].[Italy], [completion date].[ALL].[2020].[Q1].[M3] )"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with member [starting date].XXX as (prevMember(currentMember([measure])), [starting date].[2021]) " \
" select " \
" members(measure) on 1, " \
" { ( [starting date].[2021] ), ([starting date].XXX) } on 0 " \
" from [logistics.test]  " \
" where " \
" (Goods.[ALL].[household appliances].[television], Transport.[ALL].[ocean freight], [starting region].[ALL].[Europe].[UK], [ending region].[ALL].[Europe].[Italy], [starting date].[ALL].[2021].[Q2].[M4], [completion date].[ALL].[2020].[Q1].[M1]) "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" { (Calendar.[ALL].[2021].Q2.M4), (Calendar.[ALL].[2021].Q3.M7) } on 0, " \
" { (measure.[sales amount]) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" { (Calendar.[ALL].[2021].Q3.M7), (Calendar.[ALL].[2021].Q4.M10) } on 0, " \
" { (measure.[sales amount]) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with member measure.XXX as (measure.[sales amount], parallelPeriod())" \
" select " \
" { (Calendar.[ALL].[2021].Q3.M7), (Calendar.[ALL].[2021].Q4.M10) } on 0, " \
" { (measure.XXX) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" { (Calendar.[ALL].[2020].Q4.M12), (Calendar.[ALL].[2021].Q4.M12) } on 0, " \
" { (measure.[sales amount]) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with member measure.XXX as (measure.[sales amount], parallelPeriod(Calendar.year))" \
" select " \
" { (Calendar.[ALL].[2021].Q4.M12) } on 0, " \
" { (measure.XXX) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" { (Calendar.[ALL].[2021].Q4.M12) } on 1, " \
" { (measure.[sales amount], parallelPeriod(Calendar.year)) } on 0 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" { (Calendar.[ALL].[2021].Q1.M2), (Calendar.[ALL].[2021].Q3.M8), (Calendar.[ALL].[2021].Q4.M11) } on 0, " \
" { (measure.[sales amount]) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with member measure.XXX as (measure.[sales amount], parallelPeriod(Calendar.quarter, 3))" \
" select " \
" { (Calendar.[ALL].[2021].Q4.M11) } on 0, " \
" { (measure.XXX) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with member measure.XXX as (measure.[sales amount], parallelPeriod(Calendar.quarter, 0 - 2, Calendar.[ALL].[2021].Q1.M2))" \
" select " \
" { (Calendar.[ALL].[2021].Q4.M11) } on 0, " \
" { (measure.XXX) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with member measure.XXX as (measure.[sales amount], parallelPeriod(Calendar.year, 0 - 1))" \
" select " \
" { (Calendar.[ALL].[2020].Q4.M11) } on 0, " \
" { (measure.XXX) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with member measure.XXX as (measure.[sales amount], parallelPeriod(Calendar.year, 0 - 1))" \
" select " \
" { (Calendar.[ALL].[2020].Q4.M11) } on 0, " \
" { (measure.[sales amount]) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" { (Calendar.[ALL].[2021].Q4.M11) } on 0, " \
" { (measure.[sales amount]) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with member measure.AAA as (measure.[sales amount],   parallelPeriod(Calendar.month, 0 - 3)) " \
"   member measure.SSS as (measure.[sales quantity], parallelPeriod(Calendar.month, 0)) " \
"   member measure.DDD as (measure.[cash back],      parallelPeriod(Calendar.month, 0 - 5)) " \
" select " \
" { (Calendar.[ALL].[2021].[Q3].M7) } on 0, " \
" {(measure.AAA),(measure.SSS),(measure.DDD)} on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" LateralMembers(Calendar.[ALL].[2020].Q4) on 0, " \
" { (measure.[sales amount]) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" LateralMembers(measure.[sales amount]) on 1, " \
" LateralMembers(Calendar.[ALL].[2020].Q4) on 10 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"SELECT " \
" crossjoin(LATERALMEMBERS([starting region].[ALL].[Europe]), LATERALMEMBERS(measure.income)) On 10, " \
" LATERALMEMBERS([completion date].[ALL]) On 1 " \
"frOM [logistics.test] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" order(members(Calendar), (measure.[sales amount])) on 10, " \
" { (measure.[sales amount]) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" order(members(Calendar), (measure.[sales amount]), ASC) on 10, " \
" { (measure.[sales amount]) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" order(members(Calendar), (measure.[sales amount]), DESC) on 10, " \
" { (measure.[sales amount]) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" order(members(Calendar), (measure.[sales amount]), BASC) on 10, " \
" { (measure.[sales amount]) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" order(members(Calendar), (measure.[sales amount]), BDESC) on 10, " \
" { (measure.[sales amount]) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" { (measure.[sales amount]) } on 0, " \
" order(members(Calendar), (measure.[sales amount]), basc) on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" { ([Calendar].[ALL].[2021]) } on 0, " \
" topCount(crossjoin(members(measure), children([Store Type].[ALL]), lateralMembers([Payment Method].[ALL].Other)), 10, ([Calendar].[ALL].[2021])) on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select " \
" members(Goods) on 0, " \
" crossJoin(members(measure), members([ending region])) on 1 " \
"from [logistics.test] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select " \
" except(members(Goods), children(Goods.[foods])) on 1, " \
"{ " \
"  ( measure.cost ) " \
"} on 0 " \
"from [logistics.test] where ( Goods.[foods].[nut], Transport.[railway], [starting region].[Asia].[China], [starting date].[2019].[Q3].[M7], [completion date].[2019].[Q3].[M7] )"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select " \
" except(members(Goods), children(Goods.[foods])) on 1, " \
" except(members( measure ), {(measure.cost), (measure.income)}) on 0 " \
"from [logistics.test] where ( Goods.[foods].[nut], Transport.[railway], [starting region].[Asia].[China], [starting date].[2019].[Q3].[M7], [completion date].[2019].[Q3].[M7] )"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select " \
" crossJoin(members(measure), lateralMembers(Goods.[ALL].[foods])) on 1, " \
" {([starting date].[ALL].[2020])} on 0 " \
"from [logistics.test]"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with " \
" set [~~~!!!@@@...] as crossJoin({(measure.cost), (measure.quantity)}, {([Goods].[ALL].[Cell Phones & Accessories]), ([Goods].[ALL].[household appliances])}) " \
"select " \
" except(crossJoin(members(measure), lateralMembers(Goods.[ALL].[foods])), [~~~!!!@@@...]) on 1, " \
" {([starting date].[ALL].[2020])} on 0 " \
"from [logistics.test]"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with " \
" member measure.SSSSSS as sum(Ytd(), (measure.[sales amount])) " \
" select " \
" { (measure.[sales amount]), (measure.SSSSSS)} on 1, " \
" YTD() on 10 " \
" from [Online Store] " \
" where (Calendar.[ALL].[2021].Q4.M12, [Payment Method].[ALL].[Credit Card])"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with " \
" member measure.SSSSSS as sum(Ytd(), (measure.[sales amount])) " \
" select " \
" { (measure.[sales amount]), (measure.SSSSSS)} on 1, " \
" YTD(Calendar.[ALL].[2021].Q2.M5) on 10 " \
" from [Online Store] " \
" where (Calendar.[ALL].[2021].Q4.M12, [Payment Method].[ALL].[Credit Card])"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" members(measure) on 1, " \
" Descendants([starting date].[ALL].[2021], [starting date].[quarter]) on 0 " \
" from [logistics.test]  " \
" where " \
" (Goods.[ALL].[household appliances].[television], Transport.[ALL].[ocean freight], [starting region].[ALL].[Europe].[UK], [ending region].[ALL].[Europe].[Italy], [starting date].[ALL].[2021].[Q2].[M4], [completion date].[ALL].[2020].[Q1].[M1]) "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" members(measure) on 1, " \
" Descendants([starting date].[ALL].[2021], [starting date].[month]) on 0 " \
" from [logistics.test]  " \
" where " \
" (Goods.[ALL].[household appliances].[television], Transport.[ALL].[ocean freight], [starting region].[ALL].[Europe].[UK], [ending region].[ALL].[Europe].[Italy], [starting date].[ALL].[2021].[Q2].[M4], [completion date].[ALL].[2020].[Q1].[M1]) "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" members(measure) on 1, " \
" Descendants([starting date].[ALL].[2021], [starting date].[month], BEFORE) on 0 " \
" from [logistics.test]  " \
" where " \
" (Goods.[ALL].[household appliances].[television], Transport.[ALL].[ocean freight], [starting region].[ALL].[Europe].[UK], [ending region].[ALL].[Europe].[Italy], [starting date].[ALL].[2021].[Q2].[M4], [completion date].[ALL].[2020].[Q1].[M1]) "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" members(measure) on 1, " \
" Descendants([starting date].[ALL], [starting date].[quarter]) on 10 " \
" from [logistics.test]  " \
" where " \
" (Goods.[ALL].[household appliances].[television], Transport.[ALL].[ocean freight], [starting region].[ALL].[Europe].[UK], [ending region].[ALL].[Europe].[Italy], [starting date].[ALL].[2021].[Q2].[M4], [completion date].[ALL].[2020].[Q1].[M1]) "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" members(measure) on 1, " \
" Descendants([starting date].[ALL], [starting date].[quarter], BEFORE_AND_AFTER) on 10 " \
" from [logistics.test]  " \
" where " \
" (Goods.[ALL].[household appliances].[television], Transport.[ALL].[ocean freight], [starting region].[ALL].[Europe].[UK], [ending region].[ALL].[Europe].[Italy], [starting date].[ALL].[2021].[Q2].[M4], [completion date].[ALL].[2020].[Q1].[M1]) "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" members(measure) on 1, " \
" Descendants([starting date].[ALL], 20, LEAVES) on 10 " \
" from [logistics.test]  " \
" where " \
" (Goods.[ALL].[household appliances].[television], Transport.[ALL].[ocean freight], [starting region].[ALL].[Europe].[UK], [ending region].[ALL].[Europe].[Italy], [starting date].[ALL].[2021].[Q2].[M4], [completion date].[ALL].[2020].[Q1].[M1]) "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" select " \
" members(measure) on 1, " \
" Descendants([starting date].[ALL], 3) on 10 " \
" from [logistics.test]  "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with set SSSSSS as tail(" \
" crossjoin(LATERALMEMBERS([starting region].[ALL].[Europe]), LATERALMEMBERS(measure.income)) " \
")" \
"SELECT " \
" SSSSSS On 10, " \
" LATERALMEMBERS([completion date].[ALL]) On 1 " \
"frOM [logistics.test] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with set SSSSSS as tail(" \
" crossjoin(LATERALMEMBERS([starting region].[ALL].[Europe]), LATERALMEMBERS(measure.income)), 8 " \
")" \
"SELECT " \
" SSSSSS On 10, " \
" LATERALMEMBERS([completion date].[ALL]) On 1 " \
"frOM [logistics.test] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with set SSSSSS as filter(descendants(Calendar.[ALL].[2021], 1, SELF_AND_AFTER), (measure.[sales amount]) > 1) " \
" member measure.XXXXXX as (measure.[sales amount]) - 20000 " \
"select " \
"topPercent(SSSSSS, 25, (measure.[sales amount])) on 10, " \
"{(measure.XXXXXX)} on 1 " \
"from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with set SSSSSS as filter(descendants(Calendar.[ALL].[2021], 1, SELF_AND_AFTER), (measure.[sales amount]) > 1) " \
" member measure.XXXXXX as (measure.[sales amount]) - 20000 " \
"select " \
"bottomPercent(SSSSSS, 25, (measure.[sales amount])) on 10, " \
"{(measure.[sales amount])} on 1 " \
"from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with set SSSSSS as filter(descendants(Calendar.[ALL].[2021], 1, SELF_AND_AFTER), (measure.[sales amount]) > 1) " \
" member measure.XXXXXX as (measure.[sales amount]) - 20000 " \
"select " \
"topPercent(SSSSSS, 55, (measure.[sales amount])) on 10, " \
"{(measure.[sales amount])} on 1 " \
"from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with set SSSSSS as filter(descendants(Calendar.[ALL].[2021], 1, SELF_AND_AFTER), (measure.[sales amount]) > 1) " \
" member measure.XXXXXX as (measure.[sales amount]) - 18000 " \
"select " \
"bottomPercent(SSSSSS, 53, (measure.[XXXXXX])) on 10, " \
"{(measure.[XXXXXX])} on 1 " \
"from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with " \
"select members(measure) on 999, " \
"UNION(crossJoin(children(Goods.[ALL].[foods]), children([starting region].[ALL])), crossJoin(children(Goods.[ALL].[foods]), children([starting region].[ALL]))) on 111111 " \
"from [logistics.test] " \
"where ( Goods.[foods].[nut], Transport.[railway], [starting region].[Asia].[China], [starting date].[2019].[Q3].[M7], [completion date].[2019].[Q3].[M7] )"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"with " \
"select members(measure) on 999, " \
"UNION(crossJoin(children(Goods.[ALL].[foods]), children([starting region].[ALL])), crossJoin(children(Goods.[ALL].[foods]), children([starting region].[ALL])), All) on 111111 " \
"from [logistics.test] " \
"where ( Goods.[foods].[nut], Transport.[railway], [starting region].[Asia].[China], [starting date].[2019].[Q3].[M7], [completion date].[2019].[Q3].[M7] )"

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select " \
"members(measure) on 0, " \
"intersect(members(Calendar), children(Calendar.[ALL].[2021])) on 1 " \
"from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select " \
"members(measure) on 0, " \
"intersect(members(Calendar), UNION(children(Calendar.[ALL].[2021]), children(Calendar.[ALL].[2021].Q3))) on 1 " \
"from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
"select " \
"members(measure) on 0, " \
"intersect(members(Calendar), UNION(children(Calendar.[ALL].[2021]), children(Calendar.[ALL].[2021].Q1)), ALL) on 1 " \
"from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with " \
' member measure.SSSSSS as lookUpCube("logistics.test", "(measure.cost)") ' \
" select " \
" { ( measure.[sales amount] ), (measure.SSSSSS) } on 0, " \
" children(Calendar.[ALL].[2021]) on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
' with ' \
' member measure.AAAAAA as LOOKUPCUBE("Online Store", "(measure.[sales amount])") ' \
' member measure.BBBBBB as LOOKUPCUBE("Online Store", (measure.[sales amount], Calendar.[ALL].[2021].Q3)) ' \
' member measure.CCCCCC as LOOKUPCUBE([Online Store], "(measure.[sales amount], Calendar.[ALL].[2021].Q4) + (measure.[sales quantity], Calendar.[ALL].[2021].Q3)") ' \
' member measure.DDDDDD as LOOKUPCUBE([Online Store], (measure.[cash back])) ' \
' select ' \
' { (measure.cost), (measure.AAAAAA), (measure.BBBBBB), (measure.CCCCCC), (measure.DDDDDD) } on 0, ' \
' children(Goods.[ALL]) on 1 ' \
' from [logistics.test] '

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with  " \
" member measure.SSSSSS as sum({ (measure.XXX), (measure.YYY), (measure.ZZZ) }) " \
" member measure.XXX as ([measure].[sales amount]) * 10 " \
"      member measure.YYY as ((measure.XXX) + 111) " \
"      member measure.ZZZ as (measure.YYY) / 8 - 222.22 " \
" member measure.QQQ as sum({([measure].[sales amount]), ([measure].[cash back])}) " \
" member Calendar.VVV as sum({ ([Calendar].[ALL].[2020]), ([Calendar].[ALL].[2019]) })" \
" member measure.IIIIII as IIF( (measure.SSSSSS) > 2000000, 200, 404 )" \
" select " \
" { (measure.SSSSSS), (measure.IIIIII) } on 0, " \
" filter(members(Calendar), (((measure.SSSSSS) <= 30000) or ((measure.SSSSSS) >= 40000)) and (((measure.SSSSSS) <= 360000) or ((measure.SSSSSS) >= 370000))) on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with  " \
" member measure.SSSSSS as sum({ (measure.XXX), (measure.YYY), (measure.ZZZ) }) " \
" member measure.XXX as ([measure].[sales amount]) * 10 " \
"      member measure.YYY as ((measure.XXX) + 111) " \
"      member measure.ZZZ as (measure.YYY) / 8 - 222.22 " \
" member measure.QQQ as sum({([measure].[sales amount]), ([measure].[cash back])}) " \
" member Calendar.VVV as sum({ ([Calendar].[ALL].[2020]), ([Calendar].[ALL].[2019]) }) " \
" member measure.IIIIII as IIF( (measure.SSSSSS) > 2000000, 200, 404 ) " \
" member measure.FFFFFF as IIF( (measure.IIIIII) = 200, 321, 900000000 ) " \
" select " \
" { (measure.SSSSSS), (measure.IIIIII), (measure.FFFFFF) } on 0, " \
" filter(members(Calendar), (((measure.SSSSSS) <= 30000) or ((measure.SSSSSS) >= 40000)) and (((measure.SSSSSS) <= 360000) or ((measure.SSSSSS) >= 370000))) on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with " \
" member measure.AAA as CoalesceEmpty((measure.[cash back]), (measure.[sales quantity])) " \
" select " \
" { (measure.[sales quantity]), (measure.[cash back]), (measure.AAA) } on 0, " \
" { (Calendar.[ALL]) } on 1 " \
" from [Online Store] "

echo "........................................................................................... done"
sleep 1

./euclid-cli \
" with " \
" member measure.AAA as CoalesceEmpty(CoalesceEmpty((measure.[cash back]), (measure.[cash back], [Store Type].[ALL].[Platform Self-operated Store])), 8760.66) " \
" select " \
" { (measure.[sales amount]), (measure.[cash back]), (measure.AAA) } on 0, " \
" { (Calendar.[ALL]) } on 1 " \
" from [Online Store] "

echo "........................................................................................... finished"