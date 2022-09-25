netgen
======

netgen (net of nodes generator) utility is designed to help you to prepare routable Garmin maps in so called Polish Format.

A new utility called Netgen (net of nodes generator) is designed to help you to prepare routable Garmin maps in so called Polish Format (more about maps theory please read: Tricks in Polish Format). It is under GPL license and avialable on variety of platforms (especially Windows and unices). Another utility doing the same is Stan's MapRoute (see http://cgpsmapper.com/en/tool.htm ) but it is only Windows executable and requires ESRI as an input.

The main application for Netgen is to add parameters needed for routing in your maps in Polish Format (compatible with MapEdit). The biggest part is to find all intersections and mark them appropriately in pertinented roads. Roads (polylines) have to be numbered as well and marked with adequate speed parameters. This could be done automagically before MapCenter upload or self-made compilation.

During maps preparation, especially in big team, you can find that some crossroads does not connect accurately. Should it happens, roads will have no connection and routing will fail. It is of course big trouble for designers. Thus netgen has few functions to detect situations like this. First of all, some road ends may not be sticked. On demand, netgen is able to correct such unexactness. Be aware that some places have not to be sticked, so don't force too big distance to be sticked. To be sure, all common points could be mentioned as waypoints. Adding such a net to your map gives a view helpful in finding wrong crossroads.

Another method to point out wrong crossroads is to highlight dead ends. When you create crosses not carefully, one road is blind. Creating a map of such ends is a good way to find all non-routable streets. Of course you may have a lot of real dead ends on your map. To mark such places and not point them in such a list, you may flag them as a good end, using points of special type, even in MapEdit. After some trials and errors, you will give list of broken crossroads.

When you draw complicated crossroads, you may get two nodes too close each other. This case compilation, ie. on MapCenter, will fail. Netgen gives the possibility to find such places and mark them giving you a map of problematic places.

General rule for routable maps is not to use loops. In other words, any street cannot cross with itself. In special situation, the roundabout have to be composed of two semicircles. To let creator not to think about it, netgen cuts such a loops automatically. (Remark: this moment it is implemented only for one loop per road)

Uncarefull drawing may give intersections without common node. It is a mistake, but could be an real flyover (over-pass) as well. Netgen can detect such places and produce list of waypoints. The same as blind ends, you may mark places "here is real flyover" using MapEdit's points of special code. Remember that it is very complicated computing (it is about five times longer phase than compilation and about 40 times longer than other checks).

I have created Netgen to help my work with UMP-Lodz (map of my city, the second biggest one in Poland). Almost immediatelly, it was used in preparation of UMP-Warsaw (capital of Poland). Lot of features was made on their requests and suggestions (thanks to Alf/red/).

If you want to try netgen:

    get your favorite version of this tool
    run netgen -h to get help
    prepare your map as one mp file
    run netgen -R < yourmap.mp > routable.mp 2> routable.log to make your map routable
    run netgen -e 0.00002 -mc < yourmap.mp > nodes.wpt to get list of nodes that are sticked (additional search would be performed - filter all nodes with list of sticked nodes from log)
    run netgen -c -b -x < yourmap.mp > nodes.wpt to get list of suspicious nodes, and then filter it by grep -1,I nodes.wpt

