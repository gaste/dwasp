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
link(0,10).
link(0,28).
link(0,37).
link(0,8).
link(0,11).
link(0,30).
link(0,35).
link(0,28).
link(1,33).
link(1,35).
link(1,21).
link(1,8).
link(1,6).
link(1,27).
link(1,37).
link(1,8).
link(2,32).
link(2,12).
link(2,16).
link(2,27).
link(2,39).
link(2,6).
link(2,36).
link(2,10).
link(3,24).
link(3,30).
link(3,12).
link(3,37).
link(3,23).
link(3,32).
link(3,37).
link(3,4).
link(4,14).
link(4,6).
link(4,38).
link(4,21).
link(4,18).
link(4,20).
link(4,32).
link(4,11).
link(5,39).
link(5,29).
link(5,4).
link(5,26).
link(5,28).
link(5,6).
link(5,8).
link(5,34).
link(6,29).
link(6,15).
link(6,4).
link(6,17).
link(6,33).
link(6,10).
link(6,35).
link(6,7).
link(7,2).
link(7,22).
link(7,26).
link(7,3).
link(7,30).
link(7,30).
link(7,4).
link(7,35).
link(8,1).
link(8,4).
link(8,11).
link(8,26).
link(8,16).
link(8,1).
link(8,36).
link(8,34).
link(9,36).
link(9,26).
link(9,8).
link(9,3).
link(9,33).
link(9,5).
link(9,2).
link(9,38).
link(10,7).
link(10,9).
link(10,16).
link(10,15).
link(10,24).
link(10,15).
link(10,32).
link(10,31).
link(11,14).
link(11,26).
link(11,38).
link(11,39).
link(11,14).
link(11,12).
link(11,23).
link(11,0).
link(12,26).
link(12,10).
link(12,26).
link(12,34).
link(12,3).
link(12,31).
link(12,26).
link(12,37).
link(13,21).
link(13,24).
link(13,1).
link(13,27).
link(13,32).
link(13,1).
link(13,2).
link(13,30).
link(14,12).
link(14,1).
link(14,11).
link(14,13).
link(14,28).
link(14,9).
link(14,36).
link(14,15).
link(15,8).
link(15,32).
link(15,9).
link(15,35).
link(15,25).
link(15,7).
link(15,33).
link(15,16).
link(16,26).
link(16,37).
link(16,28).
link(16,29).
link(16,14).
link(16,3).
link(16,34).
link(16,6).
link(17,33).
link(17,33).
link(17,5).
link(17,5).
link(17,39).
link(17,15).
link(17,18).
link(17,33).
link(18,19).
link(18,13).
link(18,19).
link(18,15).
link(18,3).
link(18,26).
link(18,24).
link(18,30).
link(19,26).
link(19,26).
link(19,20).
link(19,1).
link(19,28).
link(19,20).
link(19,20).
link(19,8).
link(20,17).
link(20,0).
link(20,22).
link(20,13).
link(20,9).
link(20,21).
link(20,5).
link(20,10).
link(21,15).
link(21,38).
link(21,30).
link(21,2).
link(21,19).
link(21,31).
link(21,29).
link(21,24).
link(22,17).
link(22,25).
link(22,6).
link(22,16).
link(22,30).
link(22,1).
link(22,38).
link(22,17).
link(23,8).
link(23,24).
link(23,20).
link(23,3).
link(23,15).
link(23,27).
link(23,10).
link(23,27).
link(24,4).
link(24,3).
link(24,5).
link(24,16).
link(24,36).
link(24,4).
link(24,0).
link(24,25).
link(25,14).
link(25,11).
link(25,22).
link(25,1).
link(25,31).
link(25,29).
link(25,8).
link(25,28).
link(26,11).
link(26,34).
link(26,23).
link(26,15).
link(26,16).
link(26,21).
link(26,3).
link(26,35).
link(27,11).
link(27,19).
link(27,2).
link(27,28).
link(27,37).
link(27,13).
link(27,37).
link(27,12).
link(28,19).
link(28,31).
link(28,38).
link(28,17).
link(28,25).
link(28,2).
link(28,13).
link(28,8).
link(29,9).
link(29,21).
link(29,9).
link(29,20).
link(29,11).
link(29,15).
link(29,20).
link(29,36).
link(30,5).
link(30,35).
link(30,15).
link(30,15).
link(30,33).
link(30,13).
link(30,37).
link(30,13).
link(31,34).
link(31,17).
link(31,21).
link(31,25).
link(31,13).
link(31,28).
link(31,32).
link(31,22).
link(32,28).
link(32,7).
link(32,19).
link(32,5).
link(32,21).
link(32,15).
link(32,2).
link(32,19).
link(33,0).
link(33,15).
link(33,21).
link(33,36).
link(33,32).
link(33,36).
link(33,11).
link(33,34).
link(34,14).
link(34,30).
link(34,38).
link(34,14).
link(34,33).
link(34,36).
link(34,35).
link(34,27).
link(35,36).
link(35,20).
link(35,7).
link(35,20).
link(35,14).
link(35,21).
link(35,15).
link(35,36).
link(36,15).
link(36,11).
link(36,1).
link(36,21).
link(36,1).
link(36,1).
link(36,10).
link(36,20).
link(37,14).
link(37,36).
link(37,15).
link(37,39).
link(37,3).
link(37,29).
link(37,21).
link(37,23).
link(38,5).
link(38,32).
link(38,9).
link(38,35).
link(38,37).
link(38,39).
link(38,31).
link(38,6).
link(39,31).
link(39,27).
link(39,21).
link(39,19).
link(39,13).
link(39,23).
link(39,3).
link(39,27).

#hide. #show col/2.
