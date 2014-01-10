% Guess colours.

col(N,C) | n_col(N,C) :- node(N), colour(C).



% At least one color per node.

:- node(X), not colored(X).

colored(X) :- col(X,Fv1).



% Only one color per node.

:- col(N,C1), col(N,C2), C1!=C2.



% No two adjacent nodes have the same colour. 

:- link(X,Y),  X<Y, col(X,C), col(Y,C).
colour(1..4).
node(0).
node(1).
node(2).
node(3).
node(4).
node(5).
node(6).
node(7).
node(8).
node(9).
node(10).
node(11).
node(12).
node(13).
node(14).
node(15).
node(16).
node(17).
node(18).
node(19).
node(20).
node(21).
node(22).
node(23).
node(24).
node(25).
node(26).
node(27).
node(28).
node(29).
node(30).
node(31).
node(32).
node(33).
node(34).
node(35).
node(36).
node(37).
node(38).
node(39).
link(0,24).
link(0,9).
link(0,26).
link(0,2).
link(0,12).
link(0,33).
link(0,9).
link(0,9).
link(1,38).
link(1,3).
link(1,22).
link(1,25).
link(1,7).
link(1,29).
link(1,2).
link(1,32).
link(2,1).
link(2,5).
link(2,34).
link(2,18).
link(2,33).
link(2,12).
link(2,19).
link(2,34).
link(3,4).
link(3,18).
link(3,27).
link(3,25).
link(3,30).
link(3,8).
link(3,1).
link(3,0).
link(4,14).
link(4,16).
link(4,5).
link(4,14).
link(4,29).
link(4,12).
link(4,19).
link(4,12).
link(5,36).
link(5,10).
link(5,19).
link(5,6).
link(5,34).
link(5,30).
link(5,37).
link(5,37).
link(6,27).
link(6,25).
link(6,25).
link(6,8).
link(6,39).
link(6,7).
link(6,23).
link(6,13).
link(7,33).
link(7,19).
link(7,23).
link(7,28).
link(7,12).
link(7,5).
link(7,12).
link(7,16).
link(8,4).
link(8,12).
link(8,4).
link(8,38).
link(8,33).
link(8,10).
link(8,1).
link(8,12).
link(9,10).
link(9,14).
link(9,0).
link(9,13).
link(9,32).
link(9,33).
link(9,12).
link(9,20).
link(10,25).
link(10,38).
link(10,31).
link(10,15).
link(10,23).
link(10,34).
link(10,28).
link(10,15).
link(11,17).
link(11,10).
link(11,35).
link(11,12).
link(11,28).
link(11,27).
link(11,29).
link(11,8).
link(12,6).
link(12,8).
link(12,15).
link(12,25).
link(12,10).
link(12,19).
link(12,18).
link(12,37).
link(13,15).
link(13,2).
link(13,21).
link(13,7).
link(13,31).
link(13,6).
link(13,7).
link(13,26).
link(14,21).
link(14,28).
link(14,7).
link(14,15).
link(14,28).
link(14,30).
link(14,19).
link(14,9).
link(15,21).
link(15,9).
link(15,25).
link(15,23).
link(15,27).
link(15,36).
link(15,19).
link(15,34).
link(16,1).
link(16,29).
link(16,10).
link(16,29).
link(16,26).
link(16,18).
link(16,7).
link(16,21).
link(17,35).
link(17,5).
link(17,15).
link(17,16).
link(17,25).
link(17,33).
link(17,36).
link(17,9).
link(18,12).
link(18,28).
link(18,37).
link(18,9).
link(18,5).
link(18,12).
link(18,9).
link(18,38).
link(19,15).
link(19,16).
link(19,16).
link(19,6).
link(19,36).
link(19,30).
link(19,21).
link(19,15).
link(20,10).
link(20,37).
link(20,1).
link(20,17).
link(20,32).
link(20,26).
link(20,39).
link(20,30).
link(21,37).
link(21,22).
link(21,6).
link(21,4).
link(21,2).
link(21,31).
link(21,19).
link(21,5).
link(22,18).
link(22,17).
link(22,7).
link(22,4).
link(22,27).
link(22,6).
link(22,19).
link(22,29).
link(23,24).
link(23,24).
link(23,2).
link(23,7).
link(23,32).
link(23,19).
link(23,17).
link(23,9).
link(24,4).
link(24,14).
link(24,15).
link(24,33).
link(24,32).
link(24,34).
link(24,10).
link(24,10).
link(25,17).
link(25,33).
link(25,16).
link(25,24).
link(25,7).
link(25,33).
link(25,17).
link(25,33).
link(26,27).
link(26,30).
link(26,30).
link(26,19).
link(26,38).
link(26,27).
link(26,16).
link(26,36).
link(27,33).
link(27,20).
link(27,2).
link(27,7).
link(27,12).
link(27,14).
link(27,17).
link(27,13).
link(28,23).
link(28,17).
link(28,5).
link(28,7).
link(28,32).
link(28,33).
link(28,4).
link(28,23).
link(29,22).
link(29,25).
link(29,10).
link(29,13).
link(29,3).
link(29,3).
link(29,3).
link(29,8).
link(30,34).
link(30,8).
link(30,15).
link(30,14).
link(30,28).
link(30,0).
link(30,23).
link(30,36).
link(31,14).
link(31,13).
link(31,2).
link(31,9).
link(31,4).
link(31,1).
link(31,34).
link(31,39).
link(32,8).
link(32,31).
link(32,26).
link(32,8).
link(32,39).
link(32,0).
link(32,16).
link(32,2).
link(33,7).
link(33,18).
link(33,20).
link(33,5).
link(33,13).
link(33,20).
link(33,14).
link(33,17).
link(34,10).
link(34,12).
link(34,5).
link(34,33).
link(34,31).
link(34,17).
link(34,22).
link(34,10).
link(35,19).
link(35,29).
link(35,5).
link(35,14).
link(35,32).
link(35,17).
link(35,29).
link(35,32).
link(36,37).
link(36,33).
link(36,28).
link(36,11).
link(36,31).
link(36,20).
link(36,25).
link(36,37).
link(37,32).
link(37,38).
link(37,27).
link(37,16).
link(37,21).
link(37,10).
link(37,4).
link(37,11).
link(38,36).
link(38,28).
link(38,5).
link(38,39).
link(38,30).
link(38,9).
link(38,37).
link(38,37).
link(39,40).
link(39,8).
link(39,4).
link(39,23).
link(39,23).
link(39,18).
link(39,25).
link(39,21).

#hide. #show col/2.
