class Dimension:
    name: str
    olapContext = None

    def __init__(self, name, olap_context):
        self.name = name
        self.olapContext = olap_context

    def create_members(self, members_info: list):
        self.olapContext.create_members(self, members_info)
