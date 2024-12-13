class QuietTrError(Exception):
    msg: str
    _PREFIX: str

    def __init__(self, message: str):
        self._PREFIX = "*** ERROR: "
        self.msg = self._PREFIX + message

    def __str__(self) -> str:
        return self.msg


class QuIOError(QuietTrError):
    _PREFIX: str = "I/O - "

    def __init__(self, message: str):
        super().__init__(self._PREFIX + message)


class QuUnicodeError(QuietTrError):
    _PREFIX: str = "Data type mismatch - "

    def __init__(self, message: str):
        super().__init__(self._PREFIX + message)


class QuArgumentsError(QuietTrError):
    _PREFIX: str = "Invalid arguments - "

    def __init__(self, message: str):
        super().__init__(self._PREFIX + message)


class QuChecksumError(QuietTrError):
    _PREFIX: str = "Checksum error - "

    def __init__(self, message: str):
        super().__init__(self._PREFIX + message)


class QuValueError(QuietTrError):
    _PREFIX: str = "Value error - "

    def __init__(self, message: str):
        super().__init__(self._PREFIX + message)
