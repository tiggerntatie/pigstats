// PigStats for JavaScript 
// Post-parse configuration settings

// minimum player kills needed to appear in summary
// eliminates players that just dropped in for a minute or two
pscfgminkill = 1

// ignore players that have no deaths logged.. 
// eliminates players with '-1' ratio values
// set= true to ignore, false otherwise
pscfgignoreplayerswithnodeaths = true

// ignore players with default name 
// eliminates players named "player", "Sir_Knumskull", etc.
// ex:  pscfgignoreplayer = ["Sir_Knumskull","Player","cheater dude"]
pscfgignoreplayer = ["Sir_Knumskull"]

// ignore "unranked" players in cross reference views
// eliminates good ratios against occasional players
pscfigignoreunranked = false
