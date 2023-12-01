metadata_statements = (
    """create dimensions
    [Regions]
    [Dates].Calendar
    [Goods]
    [Payment Methods]
    [Customer Types]
    [Sales Channels];""",

    """create levels
    [Regions] (1:[Continents], 2:[Countries]),
    [Dates].Calendar (1:[Years], 2:[Quarters], 3:[Months]);""",

    """build cube [Andes Online Store]
dimensions
    [Dates]
    [Goods]
    [Regions]
    [Payment Methods]
    [Customer Types]
    [Sales Channels]
measures [sales] [sales count];""",

    """build cube [Sahara Online Store]
dimensions
    [Dates]
    [Goods]
    [Regions]
    [Payment Methods]
    [Customer Types]
    [Sales Channels]
measures [sales] [sales count];""",

    """create members
[Regions].[Asia].[China],
[Regions].[Asia].[India],
[Regions].[Asia].[Japan],
[Regions].[Asia].[Korea],
[Regions].[Asia].[Singapore],
[Regions].[Europe].[France],
[Regions].[Europe].[Germany],
[Regions].[Europe].[Greece],
[Regions].[Europe].[Italy],
[Regions].[Europe].[UK],
[Regions].[North America].[Canada],
[Regions].[North America].[Cuba],
[Regions].[North America].[Jamaica],
[Regions].[North America].[Mexico],
[Regions].[North America].[USA],
[Dates].[2021].[Q1].[M1],
[Dates].[2021].[Q1].[M2],
[Dates].[2021].[Q1].[M3],
[Dates].[2021].[Q2].[M4],
[Dates].[2021].[Q2].[M5],
[Dates].[2021].[Q2].[M6],
[Dates].[2021].[Q3].[M7],
[Dates].[2021].[Q3].[M8],
[Dates].[2021].[Q3].[M9],
[Dates].[2021].[Q4].[M10],
[Dates].[2021].[Q4].[M11],
[Dates].[2021].[Q4].[M12],
[Dates].[2022].[Q1].[M1],
[Dates].[2022].[Q1].[M2],
[Dates].[2022].[Q1].[M3],
[Dates].[2022].[Q2].[M4],
[Dates].[2022].[Q2].[M5],
[Dates].[2022].[Q2].[M6],
[Dates].[2022].[Q3].[M7],
[Dates].[2022].[Q3].[M8],
[Dates].[2022].[Q3].[M9],
[Dates].[2022].[Q4].[M10],
[Dates].[2022].[Q4].[M11],
[Dates].[2022].[Q4].[M12],
[Dates].[2023].[Q1].[M1],
[Dates].[2023].[Q1].[M2],
[Dates].[2023].[Q1].[M3],
[Dates].[2023].[Q2].[M4],
[Dates].[2023].[Q2].[M5],
[Dates].[2023].[Q2].[M6],
[Dates].[2023].[Q3].[M7],
[Dates].[2023].[Q3].[M8],
[Dates].[2023].[Q3].[M9],
[Dates].[2023].[Q4].[M10],
[Dates].[2023].[Q4].[M11],
[Dates].[2023].[Q4].[M12],
[Goods].[Clothing].[Ladies].[Cashmere Sweater],
[Goods].[Clothing].[Ladies].[Dress],
[Goods].[Clothing].[Ladies].[Skirt],
[Goods].[Clothing].[Ladies].[T-Shirt],
[Goods].[Clothing].[Men].[Jacket],
[Goods].[Clothing].[Men].[Jeans],
[Goods].[Clothing].[Men].[Shirt],
[Goods].[Clothing].[Men].[Suit],
[Goods].[Electronic].[Cell Phone].[Apple],
[Goods].[Electronic].[Cell Phone].[Oppo],
[Goods].[Electronic].[Cell Phone].[Samsung],
[Goods].[Electronic].[Cell Phone].[Xiaomi],
[Goods].[Electronic].[Video Games].[Arcade],
[Goods].[Electronic].[Video Games].[PS],
[Goods].[Electronic].[Video Games].[Switch],
[Goods].[Electronic].[Video Games].[Xbox],
[Goods].[Foodstuff].[Fruits].[Banana],
[Goods].[Foodstuff].[Fruits].[Durian],
[Goods].[Foodstuff].[Fruits].[Lemon],
[Goods].[Foodstuff].[Fruits].[Peach],
[Goods].[Foodstuff].[Meat].[Beef],
[Goods].[Foodstuff].[Meat].[Chicken],
[Goods].[Foodstuff].[Meat].[Fish],
[Goods].[Foodstuff].[Meat].[Pork],
[Payment Methods].[Amazon Pay],
[Payment Methods].[Apple Pay],
[Payment Methods].[Credit card],
[Payment Methods].[Debit card],
[Payment Methods].[Google Pay],
[Payment Methods].[PayPal],
[Customer Types].[Bargain hunters],
[Customer Types].[Loyal customers],
[Customer Types].[New customers],
[Customer Types].[One-time buyers],
[Customer Types].[Researchers],
[Customer Types].[Return customers],
[Sales Channels].[Affiliate marketing],
[Sales Channels].[Direct sales],
[Sales Channels].[Email marketing],
[Sales Channels].[Social media],
[Sales Channels].[Telemarketing],
[Sales Channels].[Wholesale];"""
)
