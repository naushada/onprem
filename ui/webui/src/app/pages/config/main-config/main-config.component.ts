import { Component } from '@angular/core';

@Component({
  selector: 'app-main-config',
  templateUrl: './main-config.component.html',
  styleUrls: ['./main-config.component.scss']
})
export class MainConfigComponent {
  selectedItem:string = 'CreateConfig';

  constructor() {}

  onClick(item:string) {
    this.selectedItem = item;
  }
}
