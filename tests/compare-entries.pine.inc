
// Append to the indicator in a temporary copy and open Pine Logs on a weekly
// chart. Model regular-session next opens, 5 bps per side, no leverage.
comparisonStart = input.time(timestamp("01 Jan 2021 00:00 -0500"), "Comparison start")

f_score(int side, float floorPrice) =>
    var int pendingSide = 0
    var float pendingFloor = na
    var float entry = na
    var float capital = 1.0
    var float peak = 1.0
    var float drawdown = 0.0
    var int count = 0
    var int wins = 0
    var float netSum = 0.0
    var float entryPremium = 0.0
    var float winSum = 0.0
    var float lossSum = 0.0
    if pendingSide == 1
        entry := open
        entryPremium += 100 * (open / pendingFloor - 1)
    if pendingSide == -1 and not na(entry)
        float multiple = open * 0.9995 / (entry * 1.0005)
        float pnl = 100 * (multiple - 1)
        capital *= multiple
        netSum += pnl
        count += 1
        wins += pnl > 0 ? 1 : 0
        winSum += math.max(pnl, 0)
        lossSum += math.min(pnl, 0)
        entry := na
    float equity = na(entry) ? capital : capital * close / (entry * 1.0005)
    peak := math.max(peak, equity)
    drawdown := math.max(drawdown, 100 * (1 - equity / peak))
    pendingSide := side
    pendingFloor := floorPrice
    int entries = count + (na(entry) ? 0 : 1)
    str.tostring(count) + " trades, wins " + str.tostring(wins) + ", mean " + str.tostring(netSum/count, "#.##") + "%, mean win/loss " + str.tostring(winSum/wins, "#.##") + "/" + str.tostring(lossSum/(count-wins), "#.##") + "%, DD " + str.tostring(drawdown, "#.##") + "%, entry from low " + str.tostring(entryPremium/entries, "#.##") + "%"

f_comparison() =>
    [ended, event, price, floorPrice, reclaim, phase] = f_burstCore()
    [breakSide, level] = f_reboundTrade(phase, event, floorPrice, true)
    [holdSide, unused] = f_reboundTrade(phase, event, floorPrice, false)
    var int first = na
    bool sample = time >= comparisonStart
    if sample and na(first)
        first := time
    string b = f_score(sample ? breakSide : 0, floorPrice)
    string h = f_score(sample ? holdSide : 0, floorPrice)
    var string lastTrade = "none"
    if breakSide != 0
        lastTrade := (breakSide > 0 ? "BUY" : "SELL") + " " + f_time(ended) + " @ " + f_price(price) + ", level " + f_price(level)
    "From " + f_time(first) + " to " + f_time(ended) + " | breakout " + b + " | hold " + h + " | last " + lastTrade

for symbol in array.from("NASDAQ:NVDA", "NASDAQ:MSFT", "NASDAQ:TSLA", "NASDAQ:GOOGL")
    string report = request.security(symbol, "60", f_comparison(), lookahead=barmerge.lookahead_off)
    if barstate.islastconfirmedhistory
        log.info(symbol + " " + report)
