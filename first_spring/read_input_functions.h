#pragma once

/// переместить нужно было только <iostream>, т.к. сущности из этого подключаемого файла не используются
/// а вот <string> тут используются, его необходимо оставить

std::string ReadLine();

int ReadLineWithNumber();