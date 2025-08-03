import io

class VirtualFile(io.StringIO):
    def __init__(self, initial_value='', file_id=None):
        super().__init__(initial_value)
        self._file_id = file_id

    @property
    def file_id(self):
        return self._file_id

    @property
    def file_data(self):
        # Go to the beginning, read all, then reset position
        current_position = self.tell()
        self.seek(0)
        data = self.read()
        self.seek(current_position)
        return data