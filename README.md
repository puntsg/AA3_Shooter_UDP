# AA3 Shooter 2D Online

Resumen sencillo de las partes implementadas del launcher/bootstrap:

- Login / Register
- Ranking
- Matchmaking normal y ranked

## Login / Register


```cpp
bool SendLoginRequest(string usr, string pass)
{
    if (!isConnected)
        return false;

    LoginRequestData data;
    data.username = usr;
    data.password = pass;

    sf::Packet p;
    p << LOGIN_REQUEST;
    p << data;

    return socket.send(p) == sf::Socket::Status::Done;
}
```
```cpp
void HandleLoginRequest(Client& c, sf::Packet& p)
{
    LoginRequestData data;
    p >> data;

    LoginResponseData res;
    res.success = db.LoginPlayer(data);
    res.message = res.success ? "Login correcto." : "Login incorrecto.";

    if (res.success)
    {
        c.username = data.username;
        res.playerId = c.playerId;
        res.username = data.username;
    }

    SendLoginResponse(c, res);
}
```

## Ranking
 ```cpp
void OpenRanking()
{
    state.ResetRankingState();
    state.rankingLoading = true;
    state.rankingMessage = "Cargando ranking...";

    net.SendRankingRequest(state.nickname);
}
```

```cpp
void HandleRankingRequest(Client& c, sf::Packet& p)
{
    RankingRequestData req;
    p >> req;

    bool ok = false;
    vector<RankingData> rows = db.GetRanking(req.username, ok);

    RankingResponseData res;
    res.success = ok;
    res.message = ok ? "Ranking cargado." : "No se pudo cargar el ranking.";
    res.entries = rows;

    SendRankingResponse(c, res);
}
```

## Matchmaking


```cpp
void AskNormalMatchmaking()
{
    if (state.isSearchingMatch)
        return;

    if (net.SendMatchmakingRequest(false, state.nickname, state.myGamePort))
        statusText = "Buscando partida amistosa...";
}
```

```cpp
void HandleMatchmakingRequest(Client& c, bool ranked)
{
    vector<int>& queue = ranked ? rankedQueue : normalQueue;

    RemoveClientFromQueues(c.playerId);
    queue.push_back(c.playerId);

    if (queue.size() >= 2)
        CreateMatch(queue, ranked);
}
```


```cpp
bool SendCancelMatchmakingRequest()
{
    sf::Packet p;
    p << DISCONNECT;

    if (socket.send(p) != sf::Socket::Status::Done)
        return false;

    state.isSearchingMatch = false;
    state.currentRoomId.clear();
    state.roomPlayers.clear();
    return true;
}
```



Pendiente:

- Launcher/map check
- Game Server UDP real
- GameScene del shooter, porque aún hay código legacy del 3 en raya
- Ranked por score real
