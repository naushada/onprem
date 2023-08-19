import { Component } from '@angular/core';

@Component({
  selector: 'app-dms-main',
  templateUrl: './dms-main.component.html',
  styleUrls: ['./dms-main.component.scss']
})
export class DmsMainComponent {

  selectedItem:string = 'CreateAccount';
  constructor() {

  }

  onClick(item:string) {
    this.selectedItem = item;
  }
}
