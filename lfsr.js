function shiftLeft() {
    let new0 = xor(room[bit(0)], xor(room[bit(6)],
        xor(room[bit(5)], room[bit(6)])))
    room.push(new0)
    room.removeAt(0)
}
function shiftRight() {
    let new7 = xor(room[bit(7)], xor(room[bit(5)],
        xor(room[bit(4)], room[bit(3)])))
    room.push(new7)
    room.removeAt(0)
}
let room: boolean[] = []
room = [true, true, false, false, false, true, false, false]
function bit(index: number) {
    return 7 - index
}
function xor(a: boolean, b: boolean) {
    if (a && !(b)) {
        return true
    } else if (b && !(a)) {
        return true
    } else {
        return false
    }
}
shiftRight()
console.log(room)
forever(function () {

})
